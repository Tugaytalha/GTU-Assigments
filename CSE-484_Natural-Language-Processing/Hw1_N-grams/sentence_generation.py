from n_grams import syllable_line, n_gram_to_string
from random import choice

def generate_sentence(n_gram_dict, n, syllable_or_letter):
    """
    Generate a sentence using the n-gram dictionary and the specified n-gram size.
    """
    # Sort by counts and get the most probable 5 n-grams
    sorted_n_grams = sorted(n_gram_dict.items(), key=lambda x: x[1][1], reverse=True)
    try:
        sentence = (choice(sorted_n_grams[15:20])[1][2])
    except:
        sentence = (choice(sorted_n_grams[:5])[1][2])
    while True:
        # Get the last n-1 words in the sentence
        if n == 1:
            next_word = choice(sorted_n_grams[:5])[1][0]
        else:
            n_gram = sentence[-n+1:]
            n_gram_str = ''.join(n_gram)

            # Get all n-grams starting with the last n-1 words
            possible_n_grams = {k: v for k, v in n_gram_dict.items() if k.startswith(n_gram_str)}

            # If there are no possible n-grams, select a random n-gram from the top 5
            if not possible_n_grams:
                next_word = choice(sorted_n_grams[:5])[1][0]
            else:
                # Get the one of the 5 most probable next word randomly
                next_word = choice(sorted(possible_n_grams.items(), key=lambda x: x[1][1], reverse=True)[:5])[1][0]

        # Append the next word to the sentence
        sentence.append(next_word)

        # If the next token ends with a  point or sentence gets too long, break the loop
        if next_word.endswith('.') or len(sentence) > 20:
            break


    # Join the words in the sentence
    sentence_str = ''.join(sentence)

    return sentence_str

def string_to_n_gram(s):
    # Split the string by commas
    elements = s.strip().split("<ayrim>")

    # If is there a empty string, make it a space
    for i in range(len(elements)-1):
        if elements[i] == '':
            elements[i] = ' '

    count = float(elements[-1])

    # Join the rest of the elements to form the n-gram key
    before = "".join(elements[:-2])
    after = elements[-2]

    return before, after, count, elements[:-2]


# Define the n-gram files
syllable_models = ['./models/syllable_ngram_1_smoothed', './models/syllable_ngram_2_smoothed',
                   './models/syllable_ngram_3_smoothed']
char_models = ['./models/character_ngram_1_smoothed', './models/character_ngram_2_smoothed',
                './models/character_ngram_3_smoothed']

# Generate sentences for syllable-based model
if __name__ == "__main__":
    for i, model in enumerate(syllable_models, start=1):
        n_gram_dict = {}
        with open(model, 'r', encoding='utf-8') as infile:
            j = 0
            for line in infile:
                before, after, count, bfip = string_to_n_gram(line)
                # Whileit exist in the dictionary, add a number to the key
                if before in n_gram_dict:
                    before = str(j)
                    j += 1
                if after == '<UNK>':
                    continue
                n_gram_dict[before] = (after, count, bfip)



        # Generate a sentence for the model
        sentence = generate_sentence(n_gram_dict, i, True)
        print(f"Syllable-based model with n={i} generated sentence: {sentence}")

    # Generate sentences for character-based model
    for i, model in enumerate(char_models, start=1):
        n_gram_dict = {}
        with open(model, 'r', encoding='utf-8') as infile:
            for line in infile:
                before, after, count, bfip = string_to_n_gram(line)
                if before in n_gram_dict:
                    before = str(j)
                    j += 1
                if after == '<UNK>':
                    continue
                n_gram_dict[before] = (after, count, bfip)


        # Generate a sentence for the model
        sentence = generate_sentence(n_gram_dict, i, False)
        print(f"Character-based model with n={i} generated sentence: {sentence}")