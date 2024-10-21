def clean_wiki_file(input_path, output_path):
    total_lines = 0

    # First, count the total number of lines to calculate progress
    with open(input_path, 'r', encoding='utf-8') as infile:
        for _ in infile:
            total_lines += 1

    with open(input_path, 'r', encoding='utf-8') as infile, open(output_path, 'w', encoding='utf-8') as outfile:
        for i, line in enumerate(infile, start=1):
            # Strip the line of leading/trailing whitespaces for accurate checks
            stripped_line = line.strip()

            # Skip lines that are XML tags or empty
            if not (stripped_line.startswith("<doc") or stripped_line.startswith("</doc") or not stripped_line):
                outfile.write(line)

            # Calculate progress percentage
            if i % 1000 == 0 or i == total_lines:
                progress_percentage = (i / total_lines) * 100
                print(f"Progress: {i}/{total_lines} lines processed. [{progress_percentage:.2f}% complete]")

# File paths
input_file = './data/wiki_00'
output_file = './data/wiki_cleaned'

# Clean the file with progress tracing
clean_wiki_file(input_file, output_file)
