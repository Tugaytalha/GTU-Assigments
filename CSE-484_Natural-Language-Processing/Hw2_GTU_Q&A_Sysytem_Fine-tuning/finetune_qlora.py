from transformers import AutoTokenizer, AutoModelForCausalLM
import torch
from peft import LoraConfig, TaskType
from transformers import BitsAndBytesConfig
from datasets import load_dataset
from huggingface_hub import notebook_login
from transformers import TrainingArguments
from trl import SFTTrainer


WITH_CONTEXT = False

notebook_login()

# Set device
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# --- Lora Configuration ---
lora_config = LoraConfig(
    r=64,
    lora_alpha=64,
    target_modules=["q_proj", "o_proj", "k_proj", "v_proj", "gate_proj", "up_proj", "down_proj"],
    task_type=TaskType.CAUSAL_LM,
)
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,
    bnb_4bit_quant_type="nf4",
    bnb_4bit_compute_dtype=torch.bfloat16
)

# --- Model and Tokenizer ---
name_addition = ""
loc_model = ("gtu-qa-llm-finetuned-context" if WITH_CONTEXT else "gtu-qa-llm-finetuned-no-context")
local_model = loc_model + name_addition
from_trained = True
modelName = "anilguven/Llama3.2-1b-instruct-OpenOrcaTr-unsloth" if not from_trained else "./models/" + local_model
tokenizer = AutoTokenizer.from_pretrained(modelName)
model = AutoModelForCausalLM.from_pretrained(modelName, quantization_config=bnb_config, device_map="auto")

# Move model to empty tensors
model = model.to_empty(device=device)

# --- Load and Prepare Dataset ---
dataset = load_dataset("csv", data_files="./data/qa_dataset.csv", split="train")
print("\n\n\n")
print(dataset, dataset[0])
print("\n\n\n")

alpaca_prompt_context = """
### Instruction:
Sen, Gebze Teknik Üniversitesi'nin lisans yönetmeliği hakkında sorulan soruları yanıtlamak üzere eğitilmiş bir yapay zeka asistanısın. Görev tanımın, yalnızca sağlanan bağlam belgesine dayanarak, doğru ve net cevaplar vermektir. Bağlam belgesi, GTU lisans yönetmeliğinin ilgili bölümlerinden oluşmaktadır.

1. Soru dikkatlice oku.
2. Bağlam belgesini incele ve soruyu yanıtlamak için gerekli bilgiyi bul.
3. Cevabını, yalnızca bağlam belgesinde bulunan bilgilere dayalı olarak, açık ve anlaşılır bir şekilde formüle et.
4. Eğer bağlam belgesinde sorunun cevabı yoksa, soruya cevap ver ve cevabın sonuna "Bu sorunun cevabı bağlamda bulunmamaktadır. Cevabın yanlış olma olasılığı yüksektir" yaz.
5. Asla bağlam belgesi dışındaki bilgilerden yararlanma veya tahminde bulunma.

Unutma, senin görevin GTU lisans yönetmeliği ile sınırlı.

### Input:
{}

### Output:
{}
"""

alpaca_prompt_non_context = """Below is an instruction that describes a task, paired with an input that provides further context. Write a response that appropriately completes the request.

### Instruction:
Sen, Gebze Teknik Üniversitesi'nin lisans yönetmeliği hakkında geniş bir bilgi birikimine sahip bir yapay zeka asistanısın.
1. Soruyu dikkatlice oku.
2. Soruyu en doğru ve eksiksiz şekilde yanıtla.
3. Cevabını açık ve anlaşılır bir şekilde formüle et.
4. Cevabı kesin olarak bilmiyorsan, tahminde bulun ve cevabın sonuna "Bu sorunun cevabından emin değilim." yaz
5. Cevapları Türkçe ver.

### Input:
{}

### Output:
{}
"""

alpaca_prompt = alpaca_prompt_context if WITH_CONTEXT else alpaca_prompt_non_context

eos_token = tokenizer.eos_token

tokenizer.pad_token_id = 128002
pad_token = tokenizer.pad_token

print("\n\n\n")
print(eos_token, pad_token)
print("\n\n\n")

# --- Formatting Prompts ---
def formatting_prompts_func(df):
    """
    Formats the prompts for the Alpaca model. The prompts are formatted based on the context and the question-answer pairs.
    Args:
        df: A DataFrame containing the context, question, and answers columns.

    Returns:

    """
    contexts      = df["context"]
    inputs       = df["question"]
    outputs      = df["answers"]
    texts = []

    if WITH_CONTEXT:
        for context, input, output in zip(contexts, inputs, outputs):
            text = alpaca_prompt.format(context, input, output) + eos_token
            texts.append(text)
    else:
        for input, output in zip(inputs, outputs):
            text = alpaca_prompt.format(input, output) + eos_token
            texts.append(text)

    return { "text" : texts, }

dataset = dataset.map(formatting_prompts_func, batched = True)


print("\n\n\n")
print(dataset["text"][0])
print("\n\n\n")

# --- Training Arguments ---
epoch = 6
train_args = TrainingArguments(
        per_device_train_batch_size = 3,
        gradient_accumulation_steps = 4,
        warmup_steps = 5,
        #max_steps = 150,
        num_train_epochs = epoch,
        gradient_checkpointing = True,
        learning_rate = 6e-3,
        fp16 = False,
        bf16 = True,
        logging_steps = 1,
        optim = "adamw_8bit",
        weight_decay = 0.01,
        lr_scheduler_type = "linear",
        output_dir = "outputs",
)

# --- Trainer ---
trainer = SFTTrainer(
    model = model,
    tokenizer = tokenizer,
    args = train_args,
    peft_config = lora_config,
    train_dataset = dataset
)
trainer.train()


# --- Save Model ---
username = "Tugaytalha"
finetuned_model_name = loc_model if not from_trained else local_model + "-" + str(epoch)
# Save model and tokenizer locally
model.save_pretrained("./models/" +finetuned_model_name)
tokenizer.save_pretrained("./models/" +finetuned_model_name)
# Save model and tokenizer to the Hugging Face hub
model.push_to_hub(username + "/" + finetuned_model_name)
tokenizer.push_to_hub(username + "/" + finetuned_model_name)
