from collections import defaultdict
import numpy as np
from scipy import linalg

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
    zs = np.array([2 * counts_of_counts[r] / (sorted_counts[i + 1] - sorted_counts[i - 1]) if i != 0 and i != len(
        sorted_counts) - 1 else (0 if i == 0 else 2 * counts_of_counts[r] / (sorted_counts[i + 1] - sorted_counts[i]))
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
        if abs(log_x - log_y) > t:
            r_smoothed_log[r] = log_x
        else:
            r_smoothed_log[r] = log_y

    # Normalize the smoothed probabilities in log-space and adjust for unseen n-grams
    log_total_smoothed = np.log(sum(counts_of_counts[r] * np.exp(r_smoothed_log[r]) for r in r_smoothed_log))

    for n_gram, count in n_gram_dict.items():
        if count in r_smoothed_log:
            smoothed_ngrams[n_gram] = np.log(1 - p0) + r_smoothed_log[count] - log_total_smoothed
        else:
            smoothed_ngrams[n_gram] = np.log(count / total_ngrams)  # Direct log-probability for unseen n-grams

    return smoothed_ngrams