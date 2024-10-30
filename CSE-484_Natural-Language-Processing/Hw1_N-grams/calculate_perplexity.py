import math
from n_grams import syllable_line
from smoothing import string_to_n_gram


def calculate_perplexity(n_gram_dict, smoothed_n_grams, test_file_path, n, syllable_or_letter):
    """
    Calculate the perplexity of a test set given the n-gram dictionary and smoothed n-grams.
    """
    total_log_prob = 0
    total_ngrams = 0

    with open(test_file_path, 'r', encoding='utf-8') as infile:
        for line in infile:
            parts = syllable_line(line, syllable_or_letter)

            for j in range(len(parts) - n + 1):
                n_gram = parts[j:j + n]
                n_gram_str = ' '.join(n_gram)

                # Get the smoothed probability if it exists else use <UNK> probability
                prob = smoothed_n_grams.get(n_gram_str, smoothed_n_grams['<UNK>'])

                total_log_prob += math.log(prob)
                total_ngrams += 1

    # Calculate perplexity
    perplexity = math.exp(-total_log_prob / total_ngrams)
    return perplexity

# Define the test file path
test_file_syllable = './data/wiki_syllable_test'
test_file_char = './data/wiki_character_test'

# Load the smoothed n-grams
smoothed_n_grams = {}
with open('./data/wiki_syllable_model_1_smoothed', 'r', encoding='utf-8') as infile:
    for line in infile:
        n_gram, count = string_to_n_gram(line)
        smoothed_n_grams[n_gram] = count

# Calculate perplexity for syllable-based model
if __name__ == "__main__":
    n_gram_dict = {}
    with open('./data/wiki_syllable_model_1', 'r', encoding='utf-8') as infile:
        for line in infile:
            n_gram, count = string_to_n_gram(line)
            n_gram_dict[n_gram] = count

    # Calculate perplexity
    perplexity = calculate_perplexity(n_gram_dict, smoothed_n_grams, test_file_path, 1, True)
    print(f"Syllable-based model perplexity: {perplexity}")