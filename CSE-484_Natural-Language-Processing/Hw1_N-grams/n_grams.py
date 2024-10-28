from collections import defaultdict
import math
import numpy as np
from scipy import linalg

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


def count_of_counts_table(counts):
    """
    Given a dictionary mapping n-grams to counts, return a dictionary
    mapping a count to the number of n-grams that have that count.
    """
    counts_of_counts = defaultdict(int)
    for count in counts.values():
        counts_of_counts[count] += 1
    return counts_of_counts


def log_linear_regression(counts_of_counts, sorted_counts):
    """
    Perform a log-linear regression of Z(r) on r for smoothing.
    """
    rs = np.array(sorted_counts)
    zs = np.array([2 * counts_of_counts[r] / (sorted_counts[i + 1] - sorted_counts[i - 1]) if i != 0 and i != len(
        sorted_counts) - 1 else 0
                   for i, r in enumerate(sorted_counts)])

    # Filter out zero values for log transformation
    rs = rs[zs > 0]
    zs = zs[zs > 0]

    # Perform log-linear regression: log(Z) = a * log(r) + b
    A = np.c_[np.log(rs), np.ones(len(rs))]
    coef, _, _, _ = linalg.lstsq(A, np.log(zs))
    a, b = coef
    return a, b


def simple_good_turing_smoothing(n_gram_dict, confidence_level=1.96):
    """
    Apply Simple Good-Turing smoothing to a dictionary of n-grams.
    :param n_gram_dict: Dictionary of n-grams and their frequencies
    :param confidence_level: Controls the width of confidence interval (default 1.96 for 95% confidence)
    :return: Dictionary of smoothed n-grams with probabilities
    """
    # Step 1: Count of counts table
    counts_of_counts = count_of_counts_table(n_gram_dict)
    sorted_counts = sorted(counts_of_counts.keys())

    total_ngrams = sum(n_gram_dict.values())
    p0 = counts_of_counts[1] / total_ngrams if 1 in counts_of_counts else 0

    # Step 2: Log-linear regression for Z(r) on r
    a, b = log_linear_regression(counts_of_counts, sorted_counts)

    # Step 3: Apply smoothing for n-grams
    smoothed_ngrams = {}
    r_smoothed = {}
    use_y = False

    for r in sorted_counts:
        if r == 0:  # Ignore zero counts
            continue

        # Empirical Turing estimate for r
        if r + 1 in counts_of_counts:
            x = (r + 1) * counts_of_counts[r + 1] / counts_of_counts[r]
        else:
            x = 0

        # Log-linear smoothing estimate for r
        y = (r + 1) * np.exp(a * np.log(r + 1) + b) / np.exp(a * np.log(r) + b)

        # Confidence interval for empirical Turing estimate
        if r + 1 in counts_of_counts and r in counts_of_counts:
            Nr = counts_of_counts[r]
            Nr1 = counts_of_counts[r + 1]
            t = confidence_level * np.sqrt((r + 1) ** 2 * Nr1 / Nr ** 2 * (1 + Nr1 / Nr))
        else:
            t = 0

        # Step 4: Choose between the empirical estimate and the smoothed one
        if np.abs(x - y) > t:
            r_smoothed[r] = x
        else:
            r_smoothed[r] = y

    # Normalize the smoothed probabilities and adjust for unseen n-grams
    total_smoothed = sum(counts_of_counts[r] * r_smoothed[r] for r in r_smoothed)
    for n_gram, count in n_gram_dict.items():
        smoothed_ngrams[n_gram] = (1 - p0) * (
                    r_smoothed[count] / total_smoothed) if count in r_smoothed else count / total_ngrams

    return smoothed_ngrams


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
        smoothed_n_grams = simple_good_turing_smoothing(n_gram_dict)

        # Write n-grams to the output file
        for n_gram_str, count in n_gram_dict.items():
            outfile.write(n_gram_to_string(n_gram_str, count))

        # Write smoothed n-grams to the output file
        for n_gram_str, count in smoothed_n_grams.items():
            smoothed_outfile.write(n_gram_to_string(n_gram_str, count))

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

