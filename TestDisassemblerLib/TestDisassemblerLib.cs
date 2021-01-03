/* Copyright (c) 2020 Rick (rick 'at' gibbed 'dot' us)
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 */

/*
 * This is an altered version of the original source. Functions have been
 * extracted from the original "ScriptCacheDumpTest" so they may be used as
 * a library.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Gibbed.RED4.ScriptFormats;
using Gibbed.RED4.ScriptFormats.Definitions;
using Gibbed.RED4.ScriptFormats.Instructions;
using ScriptCacheDumpTest;

namespace TestDisassemblerLib
{
	public class TestDisassemblerLib
	{
		public static void DumpFunction(CacheFile cacheFile, FunctionDefinition function, StringBuilder sb, bool validate)
		{
			var functionName = function.ToName();
			if(function.Name != functionName)
			{
				sb.AppendLine($"// {function.Name}");
			}

			sb.AppendLine(GetFunctionDeclaration(function, fullName: true));
			sb.AppendLine("{");

			const FunctionFlags ignoredFlags = FunctionFlags.HasReturnValue |
				FunctionFlags.HasParameters | FunctionFlags.HasLocals |
				FunctionFlags.HasCode;
			var flags = function.Flags & ~ignoredFlags;
			if(flags != FunctionFlags.None)
			{
				sb.AppendLine($"  // Flags : {flags}");
			}

			if(function.Locals != null && function.Locals.Count > 0)
			{
				foreach(var local in function.Locals)
				{
					sb.AppendLine($"  var {local.ToName()} : {local.Type.ToPath()};");
				}
				sb.AppendLine();
			}

			var groups = InstructionGrouper.GroupCode(cacheFile, function).ToArray();

			var groupStack = new LinkedList<(InstructionGrouper.Group group, string indent, bool isLast)>();
			for(int i = 0; i < groups.Length; i++)
			{
				groupStack.AddLast((groups[i], "", i == groups.Length - 1));
			}

			int opcodeIndex = 0;
			while(groupStack.Count > 0)
			{
				var (group, indent, isLast) = groupStack.First.Value;
				groupStack.RemoveFirst();

				var instruction = group.Instruction;

				sb.Append(' ');

				if(instruction.LoadPosition < 0)
				{
					sb.Append($" (@0x?????? ????)");
				}
				else
				{
					var loadPosition = instruction.LoadPosition;

					long absolutePosition;
					long relativePosition;
					if(validate)
					{
						absolutePosition = loadPosition + function.LoadPosition;
						relativePosition = loadPosition - function.CodeLoadPosition;
					}
					else
					{
						absolutePosition = loadPosition;
						relativePosition = loadPosition - function.CodeLoadPosition;
					}

					sb.Append($" (@0x{absolutePosition:X6} {relativePosition,4})");
				}

				sb.Append($" #{opcodeIndex++,-4}");

				sb.Append(indent);
				sb.Append(isLast == true ? " └─" : " ├─");

				DumpInstruction(instruction, sb);

				var lastChildIndex = group.Children.Count - 1;
				for(int i = lastChildIndex; i >= 0; i--)
				{
					var child = group.Children[i];
					groupStack.AddFirst((child, indent + (isLast == true ? "   " : " │ "), i == lastChildIndex));
				}
			}

			sb.AppendLine("}");
		}

		private static string GetFunctionDeclaration(FunctionDefinition function, bool withName = true, bool fullName = false)
		{
			var sb = new StringBuilder();
			if((function.Flags & FunctionFlags.IsNative) != 0)
			{
				sb.Append("native ");
			}
			if(withName == true)
			{
				sb.Append($"function {(fullName == true ? function.ToPath() : function.ToName())}");
			}
			else
			{
				sb.Append("func");
			}
			sb.Append('(');
			for(int i = 0; i < function.Parameters.Count; i++)
			{
				var parameter = function.Parameters[i];
				if(i > 0)
				{
					sb.Append(", ");
				}
				if((parameter.Flags & ParameterFlags.IsOptional) != 0)
				{
					sb.Append("optional ");
				}
				if((parameter.Flags & ParameterFlags.IsOut) != 0)
				{
					sb.Append("out ");
				}
				sb.Append($"{parameter.ToName()} : {parameter.Type.ToPath()}");
				/*var unknownParameterFlags = parameter.Flags & ~(ParameterFlags.IsOptional | ParameterFlags.IsOut);
				if (unknownParameterFlags != 0)
				{
					sb.Append($" [UNKNOWN PARAMETER FLAGS={unknownParameterFlags}]");
				}*/
			}
			sb.Append(")");
			if(function.ReturnType != null)
			{
				sb.Append($" : {function.ReturnType.ToPath()}");
			}
			return sb.ToString();
		}

		private static void DumpInstruction(Instruction instruction, StringBuilder sb)
		{
			var opName = GetOpcodeName(instruction);

			sb.Append($"{opName}");

			if(instruction.Argument == null)
			{
				sb.AppendLine();
				return;
			}

			if(instruction.Argument is string s)
			{
				sb.AppendLine($" \"{s}\"");
			}
			else if(instruction.Argument is byte[] bytes)
			{
				sb.Append(" bytes(");
				sb.Append(string.Join(", ", bytes.Select(b => "0x" + b.ToString("X2")).ToArray()));
				sb.AppendLine(")");
			}
			else if(instruction.Opcode == Opcode.EnumConst)
			{
				var (enumeration, enumeral) = (EnumConst)instruction.Argument;
				sb.AppendLine($" ({enumeration.ToPath()}, {enumeral.ToName()})");
			}
			else if(instruction.Opcode == Opcode.LocalVar)
			{
				var local = (LocalDefinition)instruction.Argument;
				sb.AppendLine($" {local.ToName()} // {local.Type.ToPath()}");
			}
			else if(instruction.Opcode == Opcode.ParamVar)
			{
				var parameter = (ParameterDefinition)instruction.Argument;
				sb.AppendLine($" {parameter.ToName()} // {parameter.Type.ToPath()}");
			}
			else if(instruction.Opcode == Opcode.ObjectVar || instruction.Opcode == Opcode.StructMember)
			{
				var property = (PropertyDefinition)instruction.Argument;
				sb.AppendLine($" {property.ToPath()} // {property.Type.ToPath()}");
			}
			else if(instruction.Opcode == Opcode.Jump ||
				instruction.Opcode == Opcode.JumpIfFalse ||
				instruction.Opcode == Opcode.Skip ||
				instruction.Opcode == Opcode.Context)
			{
				var targetIndex = (int)instruction.Argument;
				sb.AppendLine($" =>{targetIndex}");
			}
			else if(instruction.Opcode == Opcode.Switch)
			{
				var (switchType, firstCaseIndex) = (Switch)instruction.Argument;
				sb.Append($" (=>{firstCaseIndex}, {switchType.ToPath()}");
				/*if (switchType.BaseType != null)
				{
					sb.Append($" \/\* {switchType.BaseType.ToPath()} \*\/");
				}*/
				sb.AppendLine($")");
			}
			else if(instruction.Opcode == Opcode.SwitchLabel)
			{
				var (falseIndex, trueIndex) = (SwitchLabel)instruction.Argument;
				sb.AppendLine($" (false=>{falseIndex}, true=>{trueIndex})");
			}
			else if(instruction.Opcode == Opcode.FinalFunc)
			{
				var (nextIndex, sourceLine, function) = (FinalFunc)instruction.Argument;
				sb.Append($" (=>{nextIndex}, {sourceLine}, {function.ToPath()})");
				sb.Append($" // {GetFunctionDeclaration(function, false)}");
				sb.AppendLine();
			}
			else if(instruction.Opcode == Opcode.VirtualFunc)
			{
				var (nextIndex, sourceLine, name) = (VirtualFunc)instruction.Argument;
				sb.AppendLine($" (=>{nextIndex}, {sourceLine}, {name})");
			}
			else
			{
				sb.AppendLine($" {instruction.Argument}");
			}

			static string GetOpcodeName(Instruction instruction)
			{
				var name = Enum.GetName(typeof(Opcode), instruction.Opcode);
				if(name == null || name.StartsWith("Unknown") == true)
				{
					return ((byte)instruction.Opcode).ToString() + "?";
				}
				return name;
			}
		}
	}
}
