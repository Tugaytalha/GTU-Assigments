import torch
from transformers import AutoModelForCausalLM, AutoTokenizer, TrainingArguments, Trainer, DataCollatorForLanguageModeling
from peft import LoraConfig, get_peft_model, TaskType
import pandas as pd
from sklearn.model_selection import train_test_split

# --- Model and Tokenizer ---
model_name = "WiroAI/wiroai-turkish-llm-9b"
model = AutoModelForCausalLM.from_pretrained(model_name, torch_dtype=torch.bfloat16, device_map="cpu")
# Move model to empty tensors
model = model.to_empty(device=torch.device("cuda"))

tokenizer = AutoTokenizer.from_pretrained(model_name)
tokenizer.pad_token = tokenizer.eos_token # Set pad token same as eos

# --- Load and Prepare Dataset ---
df = pd.read_csv("qa_dataset.csv")

# Create prompt template
def create_prompt(row):
    prompt = f"Aşağıdaki içerik göz önüne alındığında: {row['context']}\n\nŞu soruyu yanıtlayın: {row['question']}\n\nCevap: {row['answers']}"
    return prompt

# Apply template
df["text"] = df.apply(create_prompt, axis=1)
data = df["text"].tolist()

# Tokenize and chunk data
tokenized_data = tokenizer(data, truncation=True, padding="max_length", max_length=512, return_tensors="pt")

# Split dataset
train_data, val_data = train_test_split(tokenized_data['input_ids'], test_size=0.1)

# --- LoRA Configuration ---
peft_config = LoraConfig(
    task_type=TaskType.CAUSAL_LM,
    inference_mode=False,
    r=2,  # LoRA rank
    lora_alpha=32,  # LoRA alpha
    lora_dropout=0.1,  # LoRA dropout
)

model = get_peft_model(model, peft_config)
model.print_trainable_parameters()

# --- Training Arguments ---
training_args = TrainingArguments(
    output_dir="./model/gtu-q&a-llm-9b-finetuned",
    eval_strategy="steps",
    eval_steps=100,  # Evaluate every 100 steps
    save_steps= 500,
    save_total_limit= 2,
    learning_rate=2e-4,
    per_device_train_batch_size=4,
    per_device_eval_batch_size=4,
    num_train_epochs=3,
    weight_decay=0.01,
    logging_dir="./logs",
    bf16=True,
)

# --- Data Collator ---
data_collator = DataCollatorForLanguageModeling(tokenizer=tokenizer, mlm=False)

# --- Trainer ---
trainer = Trainer(
    model=model,
    args=training_args,
    train_dataset=train_data,
    eval_dataset=val_data,
    data_collator=data_collator,
)

# --- Start Fine-tuning ---
trainer.train()

# --- Save Model ---
model.save_pretrained("./model/gtu-q&a-llm-9b-finetuned")
tokenizer.save_pretrained("./model/gtu-q&a-llm-9b-finetuned")