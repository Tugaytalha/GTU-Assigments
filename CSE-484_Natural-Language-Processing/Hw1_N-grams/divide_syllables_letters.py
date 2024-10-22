from turkishnlp import detector

# Initialize the TurkishNLP detector
nlp = detector.TurkishNLP()
nlp.download()  # Download the necessary resources for TurkishNLP
nlp.create_word_set()


# Function to syllabicate a word using TurkishNLP
def syllabicate_word(word):
    return nlp.syllabicate(word)


# Function to normalize text by converting it to lowercase
def normalize_text(text):
    return text.lower()


def process_file(input_path, syllable_output_path, char_output_path):
    # Calculate total lines for progress tracking
    total_lines = 0
    with open(input_path, 'r', encoding='utf-8') as infile:
        for _ in infile:
            total_lines += 1

    # Processing the file with progress tracking
    with open(input_path, 'r', encoding='utf-8') as infile, \
            open(syllable_output_path, 'w', encoding='utf-8') as syllable_outfile, \
            open(char_output_path, 'w', encoding='utf-8') as char_outfile:

        for i, line in enumerate(infile, start=1):
            # Normalize text by converting all letters to lowercase for character-based model
            normalized_line = normalize_text(line)
            char_outfile.write(normalized_line)

            # Break words into syllables for syllable-based model
            syllable_line = []
            words = line.strip().split()  # Split the line into words
            for word in words:
                syllables = syllabicate_word(word)
                syllable_line.append('-'.join(syllables))  # Join syllables with '-'

            # Join the syllabicated words into a new line and write to syllable-based output
            syllable_outfile.write(' '.join(syllable_line) + '\n')

            # Calculate and display progress percentage
            if i % 10000 == 0 or i == total_lines:
                progress_percentage = (i / total_lines) * 100
                print(f"Progress: {i}/{total_lines} lines processed. [{progress_percentage:.2f}% complete]")


# File paths
input_file = './data/wiki_00'
syllable_output_file = './data/wiki_syllable_model'
char_output_file = './data/wiki_character_model'

# Process the file to create both syllable-based and character-based models
process_file(input_file, syllable_output_file, char_output_file)
