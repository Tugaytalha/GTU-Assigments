import os
import re
import csv
import time
import json
from PyPDF2 import PdfReader
import google.generativeai as genai
from dotenv import load_dotenv

# Load API Key
load_dotenv()
GOOGLE_API_KEY = os.getenv("GOOGLE_API_KEY")
genai.configure(api_key=GOOGLE_API_KEY)

# --- Model Selection ---
# Try Gemini Experimental 1206 first, fallback to Gemini 2.0 Flash if not available
try:
    model = genai.GenerativeModel('gemini-experimental-1206')  # Gemini Experimental 1206 (if available)
    print("Using model: gemini-exp-1206")
except Exception as e:
    try:
        model = genai.GenerativeModel('gemini-2.0-flash-exp')  # Gemini 1.5 Flash Experimental
        print("Using model: gemini-2.0-flash-exp")
    except Exception as e:
        print(
            f"Error: Could not initialize any Gemini model.")
        exit()

# --- System Prompt ---
system_prompt = """
Sen bir soru-cevap çiftleri oluşturan bir asistansın. Sana verilen metin, Gebze Teknik Üniversitesi'nin lisans yönetmeliğinden alınmıştır.
Görev:
1. Verilen metni dikkatlice oku.
2. Metindeki her madde (MADDE) için, maddeyi ve içeriğini en iyi şekilde yansıtan 3 ila 5 arasında soru-cevap çifti oluştur.
3. Sorular açık, net ve madde içeriğine dayalı olsun.
4. Cevaplar doğrudan metinden alınsın ve doğru bilgiyi içersin.
5. Sorular ve cevaplar Türkçe dilinde olmalıdır.
Örnek:
Metin: MADDE 5 – (1) Üniversiteye bağlı fakülte ve bölümlere öğrenci kabulü, Ölçme, Seçme ve Yerleştirme Merkezi (ÖSYM) tarafından yapılan sınav sonuçlarına ve Yükseköğretim Kurulunca belirlenen esaslara göre yapılır. (2) Özel yetenek gerektiren programların sınavları Üniversite tarafından yapılır ve esasları Senato tarafından belirlenir.
Soru 1: Üniversiteye bağlı fakülte ve bölümlere öğrenci kabulü nasıl yapılır?
Cevap 1: Üniversiteye bağlı fakülte ve bölümlere öğrenci kabulü, Ölçme, Seçme ve Yerleştirme Merkezi (ÖSYM) tarafından yapılan sınav sonuçlarına ve Yükseköğretim Kurulunca belirlenen esaslara göre yapılır.
Soru 2: Özel yetenek gerektiren programların sınavları kim tarafından yapılır?
Cevap 2: Özel yetenek gerektiren programların sınavları Üniversite tarafından yapılır ve esasları Senato tarafından belirlenir.
"""


def read_pdfs(directory):
    """Reads all PDF files in a directory and returns their text content."""
    all_text = ""
    for filename in os.listdir(directory):
        if filename.endswith(".pdf"):
            try:
                reader = PdfReader(os.path.join(directory, filename))
                for page in reader.pages:
                    all_text += page.extract_text()
            except Exception as e:
                print(f"Error reading PDF {filename}: {e}")
    return all_text


def chunk_text(text):
    """Chunks the text by 'MADDE' (case-insensitive)."""
    # Add a line before the title of the madde.
    text = re.sub(r"\n(MADDE \d+)", r"\n\n\1", text)
    # Split by "MADDE" (case-insensitive) and keep delimiters
    chunks = re.split(r"(?i)(\nMADDE \d+.*?)(?=\nMADDE \d+|$)", text)
    # Remove empty strings and combine madde header with its content
    chunks = [chunks[i] + chunks[i + 1] for i in range(1, len(chunks), 2)]
    return chunks


def generate_qa_pairs(chunk, max_retries=3):
    """Generates Q&A pairs from a text chunk using the Gemini API."""
    retries = 0
    while retries < max_retries:
        try:
            messages = [
                {"role": "system", "parts": [system_prompt]},
                {"role": "user", "parts": [chunk]}
            ]

            response = model.generate_content(messages)

            # Check if response has text content
            if response.text:
                qa_pairs = []
                # Use regex to find question-answer pairs
                matches = re.findall(r"Soru \d+: (.*?)\nCevap \d+: (.*?)(?=\n|$)", response.text, re.DOTALL)
                for match in matches:
                    qa_pairs.append({
                        "question": match[0].strip(),
                        "answer": match[1].strip()
                    })
                return qa_pairs
            else:
                print(f"Warning: Empty response received from API. Retrying... (Attempt {retries + 1})")
                retries += 1
                time.sleep(2)  # Wait before retrying

        except Exception as e:
            print(f"Error during API call: {e}")
            retries += 1
            time.sleep(2)  # Wait before retrying

    print(f"Failed to generate Q&A pairs after {max_retries} retries.")
    return []


def save_to_csv(qa_data, filename="qa_dataset.csv"):
    """Saves the Q&A data to a CSV file."""
    with open(filename, "w", newline="", encoding="utf-8") as csvfile:
        fieldnames = ["id", "context", "question", "answers"]
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        for item in qa_data:
            writer.writerow(item)


if __name__ == "__main__":
    # 1. Read PDF files from the current directory
    regulations_text = read_pdfs("./data")

    # 2. Chunk the text by "MADDE"
    regulation_chunks = chunk_text(regulations_text)

    # 3. Generate Q&A pairs for each chunk
    qa_data = []
    chunk_id = 1
    for chunk in regulation_chunks:
        print(f"Processing chunk {chunk_id}...")
        qa_pairs = generate_qa_pairs(chunk)
        for qa in qa_pairs:
            qa_data.append({
                "id": chunk_id,
                "context": chunk,
                "question": qa["question"],
                "answers": json.dumps([{"text": qa["answer"], "answer_start": chunk.find(qa["answer"])}],
                                      ensure_ascii=False)  # Store answer as a JSON string
            })
        chunk_id += 1

    # 4. Save the Q&A data to a CSV file
    save_to_csv(qa_data)
    print("Q&A dataset saved to qa_dataset.csv")