from unsloth import UnslothTrainer, UnslothConfig
from transformers import AutoTokenizer, AutoModelForCausalLM
import torch
from datasets import load_dataset

# --- Set Device ---
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# --- Configuration for Unsloth ---
unsloth_config = UnslothConfig(
    model_name_or_path="anilguven/Llama3.2-1b-instruct-OpenOrcaTr-unsloth",
    task_type="causal-lm",
    quantization_config={
        "load_in_4bit": True,
        "bnb_4bit_quant_type": "nf4",
        "bnb_4bit_compute_dtype": torch.bfloat16
    },
    lora_r=16,
    lora_alpha=32,
    target_modules=["q_proj", "o_proj", "k_proj", "v_proj", "gate_proj", "up_proj", "down_proj"],
    output_dir="outputs_unsloth",
    device=device
)

# --- Load Tokenizer and Model ---
model = AutoModelForCausalLM.from_pretrained(
    unsloth_config.model_name_or_path,
    device_map="auto"
)

model = model.to(device)
tokenizer = AutoTokenizer.from_pretrained(unsloth_config.model_name_or_path)

# --- Load Dataset ---
dataset = load_dataset("csv", data_files="./data/qa_dataset.csv", split="train")

alpaca_prompt_context = """You are an AI assistant trained to answer questions about the undergraduate regulations of Gebze Technical University. **Your task is to provide accurate and clear answers solely based on the provided context document.** The context document consists of relevant sections of GTU undergraduate regulations.

### Instructions:
1. **Read the question carefully.**
2. **Review the context document to find the necessary information to answer the question.**
3. **Formulate your answer clearly and concisely, based solely on the information in the context document.**
4. **If the answer to the question is not in the context document, provide an answer and add \"The answer to this question is not found in the context. There is a high likelihood the answer might be incorrect.\" at the end of your response.**
5. **Never use information outside the context document or make assumptions.**

**Remember, your task is limited to GTU undergraduate regulations.**

### Context:
{}

### Question:
{}

### Answer:
{}
"""

# --- Formatting Prompts ---
def formatting_prompts_func(df):
    """
    Formats the prompts for the Alpaca model. The prompts are formatted based on the context and the question-answer pairs.
    Args:
        df: A DataFrame containing the context, question, and answers columns.

    Returns:

    """
    contexts = df["context"]
    inputs = df["question"]
    outputs = df["answers"]
    texts = []

    for context, input, output in zip(contexts, inputs, outputs):
        text = alpaca_prompt_context.format(context, input, output) + tokenizer.eos_token
        texts.append(text)

    return {"text": texts}

dataset = dataset.map(formatting_prompts_func, batched=True)

# --- Unsloth Trainer ---
trainer = UnslothTrainer(
    model=model,
    tokenizer=tokenizer,
    train_dataset=dataset,
    config=unsloth_config
)

# --- Train ---
trainer.train()

# --- Save Model ---
username = "Tugaytalha"
finetuned_model_name = "gtu-qa-llm-9b-finetuned-unsloth"
model.save_pretrained(f"./models/{finetuned_model_name}")
tokenizer.save_pretrained(f"./models/{finetuned_model_name}")

# --- Push to Hugging Face Hub ---
model.push_to_hub(username + "/" + finetuned_model_name)
tokenizer.push_to_hub(username + "/" + finetuned_model_name)
