from transformers import AutoTokenizer, AutoModelForCausalLM
import torch
from peft import LoraConfig, TaskType
from transformers import BitsAndBytesConfig
from datasets import load_dataset
from huggingface_hub import notebook_login

WITH_CONTEXT = False

notebook_login()

# Set device
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# --- Lora Configuration ---
lora_config = LoraConfig(
    r=16,
    lora_alpha=32,
    target_modules=["q_proj", "o_proj", "k_proj", "v_proj", "gate_proj", "up_proj", "down_proj"],
    task_type=TaskType.CAUSAL_LM,
)
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,
    bnb_4bit_quant_type="nf4",
    bnb_4bit_compute_dtype=torch.bfloat16
)

# --- Model and Tokenizer ---
modelName = "emre570/llama3.2-1b-tr-qlora"

tokenizer = AutoTokenizer.from_pretrained(modelName)
model = AutoModelForCausalLM.from_pretrained(modelName, quantization_config=bnb_config, device_map="auto")

# Move model to empty tensors
model = model.to_empty(device=device)

# --- Load and Prepare Dataset ---
dataset = load_dataset("csv", data_files="./data/qa_dataset.csv", split="train")
print("\n\n\n")
print(dataset, dataset[0])
print("\n\n\n")

alpaca_prompt_context = """Sen, Gebze Teknik Üniversitesi'nin lisans yönetmeliği hakkında sorulan soruları yanıtlamak üzere eğitilmiş bir yapay zeka asistanısın. **Görev tanımın, yalnızca sağlanan bağlam belgesine dayanarak, doğru ve net cevaplar vermektir.** Bağlam belgesi, GTU lisans yönetmeliğinin ilgili bölümlerinden oluşmaktadır.

**Talimatlar:**

1. **Soru dikkatlice oku.**
2. **Bağlam belgesini incele ve soruyu yanıtlamak için gerekli bilgiyi bul.**
3. **Cevabını, yalnızca bağlam belgesinde bulunan bilgilere dayalı olarak, açık ve anlaşılır bir şekilde formüle et.**
4. **Eğer bağlam belgesinde sorunun cevabı yoksa, soruya cevap ver ve cevabın sonuna "Bu sorunun cevabı bağlamda bulunmamaktadır. Cevabın yanlış olma olasılığı yüksektir" yaz.**
5. **Asla bağlam belgesi dışındaki bilgilerden yararlanma veya tahminde bulunma.**

**Unutma, senin görevin GTU lisans yönetmeliği ile sınırlı.**

---

### Bağlam:
```
{}
```

### Soru:
```
{}
```

### Cevap:
```
{}
```
"""

alpaca_prompt_non_context = """Sen, Gebze Teknik Üniversitesi'nin lisans yönetmeliği de dahil olmak üzere geniş bir bilgi birikimine sahip bir yapay zeka asistanısın. **Soruları, yalnızca kendi dahili bilgilerini kullanarak yanıtlamakla görevlisin. Herhangi bir harici belge veya bağlam sağlanmayacaktır.**

**Talimatlar:**

1. **Soruyu dikkatlice oku.**
2. **Kendi bilgi birikimini kullanarak soruyu en doğru ve eksiksiz şekilde yanıtla.**
3. **Cevabını açık ve anlaşılır bir şekilde formüle et.**
4. **Eğer sorunun cevabını kesin olarak bilmiyorsan, tahminde bulunma veya bilgi uydurma. Bunun yerine, "Bu sorunun cevabını bilmiyorum." yaz.**
5. **GTU lisans yönetmeliği hakkındaki sorulara, yönetmelik güncel ve doğruymuş gibi cevap vermeye çalış.**

**Unutma, soruları yanıtlarken yalnızca kendi dahili bilgilerini kullanmalısın.**

---

### Soru:
```
{}
```

### Cevap:
```
{}
```
"""

alpaca_prompt = alpaca_prompt_context if WITH_CONTEXT else alpaca_prompt_non_context

eos_token = tokenizer.eos_token

tokenizer.pad_token_id = 128002
pad_token = tokenizer.pad_token

print("\n\n\n")
print(eos_token, pad_token)
print("\n\n\n")



