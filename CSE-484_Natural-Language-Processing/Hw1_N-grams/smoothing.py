from collections import defaultdict
import numpy as np
from scipy import linalg
from n_grams import n_gram_to_string

def count_of_counts_table(counts):
    """
    Given a dictionary mapping n-grams to counts, return a dictionary
    mapping a count to the number of n-grams that have that count.
    """
    counts_of_counts = defaultdict(int)
    for count in counts.values():
        counts_of_counts[count] += 1
    return counts_of_counts


# Function to perform log-linear regression on the counts of counts
def log_linear_regression(counts_of_counts, sorted_counts):
    rs = np.array(sorted_counts)
    zs = np.array([2 * counts_of_counts[r] / (
            (sorted_counts[i + 1] if (i != len(sorted_counts) - 1) else 2 * r - sorted_counts[i - 1]) - (
                sorted_counts[i - 1] if i > 0 else 0))
                   for i, r in enumerate(sorted_counts)])
    # Filter out zero values for log transformation
    rs = rs[zs > 0]
    zs = zs[zs > 0]

    # Perform log-linear regression: log(Z) = a * log(r) + b
    A = np.c_[np.log(rs), np.ones(len(rs))]
    coef, _, _, _ = linalg.lstsq(A, np.log(zs))
    a, b = coef
    return a, b


# Function to apply simple Good-Turing smoothing to n-grams
def simple_good_turing_smoothing(n_gram_dict, confidence_level=1.96):
    """
    :param n_gram_dict: Dictionary of n-grams and their frequencies
    :param confidence_level: Controls the width of confidence interval (default 1.96 for 95% confidence)
    :return: Dictionary of smoothed n-grams with log probabilities
    """
    # Step 1: Count of counts table
    counts_of_counts = count_of_counts_table(n_gram_dict)
    sorted_counts = sorted(counts_of_counts.keys())

    # Calculate p0 for unseen n-grams
    total_ngrams = sum(n_gram_dict.values())
    p0 = counts_of_counts[1] / total_ngrams if 1 in counts_of_counts else 0

    # Step 2: Log-linear regression for Z(r) on r
    a, b = log_linear_regression(counts_of_counts, sorted_counts)

    # Step 3: Apply smoothing for n-grams
    smoothed_ngrams = {}
    r_smoothed_log = {}
    use_y = False

    for r in sorted_counts:
        if r == 0:  # Ignore zero counts
            continue

        # Empirical Turing estimate for r (in logarithmic form)
        if r + 1 in counts_of_counts:
            x = (r + 1) * counts_of_counts[r + 1] / counts_of_counts[r]
            log_x = np.log(x) if x > 0 else -np.inf
        else:
            log_x = -np.inf

        # Log-linear smoothing estimate for r
        y = (r + 1) * np.exp(a * np.log(r + 1) + b) / np.exp(a * np.log(r) + b)
        log_y = np.log(y) if y > 0 else -np.inf

        # Confidence interval for empirical Turing estimate (logarithmic space)
        if r + 1 in counts_of_counts and r in counts_of_counts:
            Nr = counts_of_counts[r]
            Nr1 = counts_of_counts[r + 1]
            t = confidence_level * np.sqrt((r + 1) ** 2 * Nr1 / Nr ** 2 * (1 + Nr1 / Nr))
        else:
            t = 0

        # Step 4: Choose between the empirical estimate and the smoothed one based on log-space confidence intervals
        if not use_y and abs(log_x - log_y) > t and log_x != -np.inf:
            r_smoothed_log[r] = log_x
        else:
            r_smoothed_log[r] = log_y

    # Normalize the smoothed probabilities in log-space and adjust for unseen n-grams
    log_total_smoothed = np.log(sum(counts_of_counts[r] * np.exp(r_smoothed_log[r]) for r in r_smoothed_log))

    print("Total smoothed log: ", log_total_smoothed)

    for n_gram, count in n_gram_dict.items():
        if count in r_smoothed_log:
            smoothed_ngrams[n_gram] = np.exp(np.log(1 - p0) + r_smoothed_log[count] - log_total_smoothed)
        else:
            smoothed_ngrams[n_gram] = np.exp(np.log(count / total_ngrams))  # Direct log-probability for unseen n-grams

    # Add unseen n-grams to the smoothed n-grams
    smoothed_ngrams['<UNK>'] = p0

    return smoothed_ngrams

def string_to_n_gram(s):
    """
    Given a string s, return a list of n-grams of size n.
    """
    # Split the string by commas
    elements = s.strip().split("<ayrim>")

    # Take last element as count
    count = float(elements[-1])

    # If is there a empty string, make it a space
    for i in range(len(elements) - 1, 0, -1):
        if elements[i] == '' and elements[i - 1] == '':
            elements[i - 1] = ' '
            elements.pop(i)

    # Join the rest of the elements to form the n-gram key
    n_gram = " ".join(elements[:-1])

    return n_gram, count


def smooth_and_save_n_gram(n_gram_path, outputh_path=None, confidence_level=1.96):
    n_gram_dict = {}
    with open(n_gram_path, 'r', encoding='utf-8') as infile:
        for line in infile:
            n_gram, count = string_to_n_gram(line)
            n_gram_dict[n_gram] = count

    # Perform simple Good-Turing smoothing
    smoothed_n_grams = simple_good_turing_smoothing(n_gram_dict, confidence_level)

    # Print sum of smoothed n-grams
    print("Sum of smoothed n-grams: ", sum(smoothed_n_grams.values()))

    if outputh_path is None:
        outputh_path = n_gram_path + "_smoothed"

    with open(outputh_path, 'w', encoding='utf-8') as outfile:
        for n_gram, count in smoothed_n_grams.items():
            outfile.write(n_gram_to_string(n_gram, count))

    return outputh_path


def smooth_all_n_grams(syllable_path, char_path):
    outputs = []

    # Smooth syllable-based n-grams
    print("Smoothing syllable-based n-grams...")
    outputs.append(smooth_and_save_n_gram(syllable_path+"_1"))
    outputs.append(smooth_and_save_n_gram(syllable_path+"_2"))
    outputs.append(smooth_and_save_n_gram(syllable_path+"_3"))

    # Smooth character-based n-grams
    print("Smoothing character-based n-grams...")
    outputs.append(smooth_and_save_n_gram(char_path+"_1"))
    outputs.append(smooth_and_save_n_gram(char_path+"_2"))
    outputs.append(smooth_and_save_n_gram(char_path+"_3"))

    return outputs

# Output paths for n-grams
syllable_output_base = './models/syllable_ngram'
char_output_base = './models/character_ngram'

# Smooth n-grams
if __name__ == '__main__':
    print(smooth_all_n_grams(syllable_output_base, char_output_base))