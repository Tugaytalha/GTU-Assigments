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
    return ','.join(n_gram_str_list) + f",{count}\n"


# Function to calculate perplexity
def calculate_perplexity(n_gram_dict, smoothed_n_grams, test_file_path, n, syllable_or_letter):
    total_log_prob = 0
    total_ngrams = 0

    with open(test_file_path, 'r', encoding='utf-8') as infile:
        for line in infile:
            parts = syllable_line(line, syllable_or_letter)

            for j in range(len(parts) - n + 1):
                n_gram = parts[j:j + n]
                n_gram_str = ' '.join(n_gram)

                # Get the smoothed probability of the n-gram
                count = smoothed_n_grams.get(n_gram_str, 0)
                total_count = sum(smoothed_n_grams.values())

                if count > 0:
                    prob = count / total_count
                else:
                    prob = 1 / total_count  # Backoff for unseen n-grams

                total_log_prob += math.log(prob)
                total_ngrams += 1

    # Calculate perplexity
    perplexity = math.exp(-total_log_prob / total_ngrams)
    return perplexity

# Function to apply Good-Turing smoothing to n-grams
def good_turing_smoothing(n_gram_dict):
    # Count how many n-grams have a specific count (frequency of frequencies)
    freq_of_freq = defaultdict(int)

    for count in n_gram_dict.values():
        freq_of_freq[count] += 1

    smoothed_n_grams = {}
    N = sum(n_gram_dict.values())  # Total count of all n-grams

    for n_gram, count in n_gram_dict.items():
        if count + 1 in freq_of_freq:
            k = count
            k_plus_1 = count + 1
            n_k = freq_of_freq[k]
            n_k_plus_1 = freq_of_freq[k_plus_1]

            # Apply Good-Turing smoothing formula
            k_star = (k_plus_1 * n_k_plus_1) / n_k if n_k != 0 else k
            smoothed_n_grams[n_gram] = k_star
        else:
            # If no n_k+1, leave it as is (no smoothing)
            smoothed_n_grams[n_gram] = count

    return smoothed_n_grams


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
            open(output_path, 'w', encoding='utf-8') as outfile, \
            open(output_path + "_smoothed", 'w', encoding='utf-8') as smoothed_outfile:

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

        # Apply Good-Turing smoothing to the n-gram counts
        smoothed_n_grams = good_turing_smoothing(n_gram_dict)

        # Write n-grams to the output file
        for n_gram_str, count in n_gram_dict.items():
            outfile.write(n_gram_to_string(n_gram_str, count))

        # Write smoothed n-grams to the output file
        for n_gram_str, count in smoothed_n_grams.items():
            outfile.write(n_gram_to_string(n_gram_str, count))
