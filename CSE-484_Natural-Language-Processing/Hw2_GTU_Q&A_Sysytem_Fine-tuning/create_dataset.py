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
