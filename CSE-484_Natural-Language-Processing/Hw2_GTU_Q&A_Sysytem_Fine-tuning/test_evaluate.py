from transformers import AutoTokenizer, AutoModelForCausalLM
import torch

WITH_CONTEXT = False

# Define model and tokenizer paths
model_name = "gtu-qa-llm-finetuned-context" if WITH_CONTEXT else "gtu-qa-llm-finetuned-no-context"
model_name = f"./models/{model_name}"

# Load the tokenizer
tokenizer = AutoTokenizer.from_pretrained(model_name)

# Load the model
model = AutoModelForCausalLM.from_pretrained(
    model_name,
    torch_dtype=torch.float16,  # Use float16 for reduced memory usage (if supported)
    device_map="auto"          # Automatically assign layers to available devices (e.g., GPU)
)

# Define input text
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
input_texts = ["Teorik dersler için devam zorunluluğu % kaçtır?", "Staj komisyonu gerekli gördüğü durumlarda ne gibi işlemler yapabilir?", "Diploma numaraları her yıl sıfırlanır mı?", "Hangi durumlarda yarıyıl içinde izin verilebilir?"]

for input_text in input_texts:
    # Tokenize input
    inputs = tokenizer(input_text, return_tensors="pt").to(model.device)


    # Generate text
    outputs = model.generate(
        inputs["input_ids"],
        max_length=100,
        num_return_sequences=1,
        # do_sample=False,
        temperature=0.6,
        top_k=50,
        top_p=0.95,
        pad_token_id=tokenizer.eos_token_id,
        bos_token_id=tokenizer.bos_token_id,
        eos_token_id=tokenizer.eos_token_id,
        no_repeat_ngram_size=2,
        num_beams=5,
        num_beam_groups=1,
        diversity_penalty=0.0,
        length_penalty=0.6,
        early_stopping=True,
        use_cache=True,
    )

    # Decode and print the generated text
    for i, output in enumerate(outputs):
        generated_text = tokenizer.decode(output, skip_special_tokens=True)
        print(f"Generated Text for input '{input_text}': \n{generated_text}\n\n")

