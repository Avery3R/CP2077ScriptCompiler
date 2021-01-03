using System;
using System.IO;
using Gibbed.RED4.ScriptFormats;
using AR = Avery3R.RED4Scripts;
using ScriptDecompilerLib;
using System.Text;

namespace ScriptCompileTester
{
	class Program
	{
		static void Main(string[] args)
		{
			//AR.Compiler.Class1.doTheThing();
			if(args.Length == 0)
			{
				Console.WriteLine("ScriptCompileTester called with <= 1 args, running internal testing code");
				//AR.Compiler.InternalTestClass.doTheThing();
				ScriptDecompilerLib.Class1.doTheThing();
			}
			else if(args.Length == 3 && args[0] == "compile")
			{
				var cacheFileBytes = File.ReadAllBytes(args[1]);
				var cacheFileStream = new MemoryStream(cacheFileBytes, false);
				var cacheFile = CacheFile.Load(cacheFileStream, true);

				string script = File.ReadAllText(args[2]);
				var parserResult = AR.Compiler.NetAPI.ParseScript(script);

				foreach(var import in parserResult.Imports)
				{
					Console.WriteLine(String.Format("Adding import to cache: {0}", import.PrettyPrint()));
					AR.Compiler.NetAPI.AddDefToCacheNoDupes(cacheFile, import.GetDefinition(cacheFile));
				}

				foreach(var func in parserResult.Functions)
				{
					Console.WriteLine(String.Format("Adding func to cache: {0}", func.PrettyPrint()));
					var funcDef = func.GetDefinition(cacheFile);
					AR.Compiler.NetAPI.AddFuncToCache(cacheFile, funcDef);

					Console.WriteLine("Disassembly: ");
					var sb = new StringBuilder();
					TestDisassemblerLib.TestDisassemblerLib.DumpFunction(cacheFile, funcDef, sb, false);
					Console.WriteLine(sb.ToString());
				}

				foreach(var parsedClass in parserResult.Classes)
				{
					Console.WriteLine(String.Format("Adding class to cache: {0}", parsedClass.PrettyPrint()));
					AR.Compiler.NetAPI.AddClassToCache(cacheFile, parsedClass.GetDefinition(cacheFile));
				}
			}
			else
			{
				Console.WriteLine("ScriptCompileTester called with invalid args. Usage: ScriptCompileTester compile <cache file path> <script file path>");
			}
		}
	}
}
