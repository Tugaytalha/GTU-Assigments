import os

def split_data(input_path, train_output_path, test_output_path, split_ratio=0.95):
    # First, count the total number of lines
    total_lines = 0
    with open(input_path, 'r', encoding='utf-8') as infile:
        for _ in infile:
            total_lines += 1

    # Calculate the number of training lines based on the split ratio
    train_size = int(total_lines * split_ratio)

    # Open files for writing the train and test sets
    with open(input_path, 'r', encoding='utf-8') as infile, \
         open(train_output_path, 'w', encoding='utf-8') as train_file, \
         open(test_output_path, 'w', encoding='utf-8') as test_file:

        for i, line in enumerate(infile, start=1):
            if i <= train_size:
                train_file.write(line)
            else:
                test_file.write(line)

            # Display progress every 1000 lines or at the end
            if i % 10000 == 0 or i == total_lines:
                progress_percentage = (i / total_lines) * 100
                print(f"Progress: {i}/{total_lines} lines processed. [{progress_percentage:.2f}% complete]")

# File paths for the syllable-based and character-based models
syllable_input_file = './data/wiki_syllable_model'
char_input_file = './data/wiki_character_model'

# Output paths for training and test sets
syllable_train_file = './data/wiki_syllable_train'
syllable_test_file = './data/wiki_syllable_test'
char_train_file = './data/wiki_character_train'
char_test_file = './data/wiki_character_test'

# Split syllable-based model
print("Splitting syllable-based model...")
split_data(syllable_input_file, syllable_train_file, syllable_test_file)

# Split character-based model
print("Splitting character-based model...")
split_data(char_input_file, char_train_file, char_test_file)
