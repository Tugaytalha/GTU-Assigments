from collections import defaultdict
import math

# Function to split a line into parts
def syllable_line(line, syllable_or_letter):
    '''
    :param line: line to be split
    :param syllable_or_letter: True if syllable, False if letter
    :return: list of syllables or letters
    '''
    # Split line by - and return syllables
    if syllable_or_letter:
        return line.strip().split('-')
    # Split line letter by letter and return as list
    else:
        return list(line.strip())


# Function to convert n-gram key and value to a comma separated string to write to a file
def n_gram_to_string(n_gram_string, count):
    n_gram_str_list = n_gram_string.split(" ")
    for i in range(len(n_gram_str_list) - 1, 0, -1):
        # If there are consecutive empty strings, it means there is a space element
        if n_gram_str_list[i] == '' and n_gram_str_list[i - 1] == '':
            n_gram_str_list[i - 1] = ' '
            n_gram_str_list.pop(i)
    # Pop empty elements
    n_gram_str_list = [x for x in n_gram_str_list if x != '']
    return '<ayrim>'.join(n_gram_str_list) + f"<ayrim>{count}\n"


# Function to create n-grams from a cleaned dataset and save them to a new file
def create_n_gram(input_path, output_path, n, syllable_or_letter):
    '''
    :param input_path: input file path
    :param output_path: output file path
    :param n: n-gram size
    :param syllable_or_letter: True if syllable, False if letter
    :save: n-gram to output_path
    :return: saved n-gram file path if successful, None otherwise
    '''
    # Calculate total lines for progress tracking
    total_lines = 0
    with open(input_path, 'r', encoding='utf-8') as infile:
        for _ in infile:
            total_lines += 1

    # Create n-gram from the file with progress tracking
    with open(input_path, 'r', encoding='utf-8') as infile, \
            open(output_path, 'w', encoding='utf-8') as outfile:

        n_gram_dict = defaultdict(int)

        for i, line in enumerate(infile, start=1):
            # Split the line into syllables or letters
            parts = syllable_line(line, syllable_or_letter)

            # Create n-grams from the parts
            for j in range(len(parts) - n + 1):
                n_gram = parts[j:j + n]
                n_gram_str = ' '.join(n_gram)

                # Add n-gram to dictionary
                n_gram_dict[n_gram_str] += 1

            # Calculate and display progress percentage
            if i % 100000 == 0 or i == total_lines:
                progress_percentage = (i / total_lines) * 100
                print(f"Progress: {i}/{total_lines} lines processed. [{progress_percentage:.2f}% complete]")

        # # Apply Good-Turing smoothing to the n-gram counts
        # smoothed_n_grams = simple_good_turing_smoothing(n_gram_dict)

        # Write n-grams to the output file
        for n_gram_str, count in n_gram_dict.items():
            outfile.write(n_gram_to_string(n_gram_str, count))

    return output_path

def create_all_n_grams(syllable_input_path, char_input_path, syllable_output_base, char_output_base):
    outputs = []

    # Create syllable-based n-grams
    print("Creating syllable-based n-grams...")
    outputs.append(create_n_gram(syllable_input_path, syllable_output_base + "_1", 1, True))
    outputs.append(create_n_gram(syllable_input_path, syllable_output_base + "_2", 2, True))
    outputs.append(create_n_gram(syllable_input_path, syllable_output_base + "_3", 3, True))

    # Create character-based n-grams
    print("Creating character-based n-grams...")
    outputs.append(create_n_gram(char_input_path, char_output_base + "_1", 1, False))
    outputs.append(create_n_gram(char_input_path, char_output_base + "_2", 2, False))
    outputs.append(create_n_gram(char_input_path, char_output_base + "_3", 3, False))

    return outputs

# File paths for the syllable-based and character-based models
syllable_input_file = './data/wiki_syllable_train'
char_input_file = './data/wiki_character_train'

# Output paths for n-grams
syllable_output_base = './models/syllable_ngram'
char_output_base = './models/character_ngram'

if __name__ == '__main__':
    # Create n-grams for syllable-based and character-based models
    n_gram_files = create_all_n_grams(syllable_input_file, char_input_file, syllable_output_base, char_output_base)

    # Print the paths of the saved n-gram files
    for file in n_gram_files:
        if file:
            print(f"Saved n-gram file: {file}")
        else:
            print("Failed to save n-gram file.")

