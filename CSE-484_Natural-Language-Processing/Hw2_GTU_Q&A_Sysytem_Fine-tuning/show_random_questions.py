import random as r
import pandas as pd

# Load the dataset
df = pd.read_csv("qa_dataset.csv")

# Show random questions
for i in range(5):
    idx = r.randint(0, len(df) - 1)
    print(f"Question: {df.iloc[idx]['question']}")
    print(f"Answer: {df.iloc[idx]['answers']}")
    print()