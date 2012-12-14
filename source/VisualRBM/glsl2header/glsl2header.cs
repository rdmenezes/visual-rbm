using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace glsl2header
{
	class glsl2header
	{
		static void Main(string[] args)
		{
			if (args.Length != 2)
			{
				Console.WriteLine("glsl2header DIRECTORY OUTPUT.H");
				Console.WriteLine("  DIRECTORY   Location of all the .vert and .frag files");
				Console.WriteLine("  OUTPUT.h    Save destination for output header file");
				return;
			}

			string directory = args[0];
			string header = args[1];

			string[] shader_files = Directory.GetFiles(directory);
			string[] shader_files_no_extension = shader_files.Select(filename => Path.GetFileNameWithoutExtension(filename)).ToArray();

			int shader_count = shader_files.Length;

			StringBuilder sb = new StringBuilder();
			sb.AppendLine("#pragma once").AppendLine();

			for(int i = 0; i < shader_count; i++)
			{
				sb.Append("const char ").Append(shader_files_no_extension[i]).Append("[] = ");

				using (StreamReader sr = new StreamReader(shader_files[i]))
				{
					for (string line = sr.ReadLine(); line != null; line = sr.ReadLine())
					{
						if (line.Trim() == "")
							continue;

						sb.AppendLine();
						sb.Append("\t");
						sb.Append(("\"" + line.Replace('\t', ' ')).Trim());
						sb.Append("\\n\"");
					}
					sb.AppendLine(";").AppendLine();
				}

				Console.WriteLine(sb.ToString());
			}

			using (StreamWriter sw = new StreamWriter(header))
			{
				sw.Write(sb.ToString());
			}
		}
	}
}
