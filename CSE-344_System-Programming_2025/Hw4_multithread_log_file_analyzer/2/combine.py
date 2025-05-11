import os

output_file = "combined_code.txt"
extensions = (".c", ".h")

with open(output_file, "w", encoding="utf-8") as outfile:
    for filename in os.listdir("."):
        if filename.endswith(extensions):
            with open(filename, "r", encoding="utf-8", errors="ignore") as infile:
                outfile.write(f"// Start of {filename}\n")
                outfile.write(infile.read())
                outfile.write(f"\n// End of {filename}\n\n")

print(f"All .c and .h files have been combined into '{output_file}'.")
