---

**Important Note on Code Display (VS Code vs. PyCharm/Colab):**

You mentioned that some comments or parts of the file might not display correctly in VS Code compared to PyCharm.

*   **Google Colab:** Colab inherently uses a cell-based structure. Each block of code or text you see in the Colab interface *is* a cell. The notebook file (`.ipynb`) you will upload is already structured into these cells (code and text/markdown). Colab will render these cells correctly, including any markdown formatting (`# %% [markdown]` markers within the code are less relevant here as the structure is defined by the notebook format itself).

The instructions below guide you on how to open and run your assignment notebook in Colab.

---

## How to Run the Assignment Code in Google Colab: Step-by-Step

**Prerequisites:**

1.  A Google Account.
2.  The assignment's Python notebook file (e.g., `Assignment.ipynb`) containing the code provided earlier.

**Steps:**

1.  **Access Google Colab:**
    *   Go to [https://colab.research.google.com/](https://colab.research.google.com/) in your web browser.
    *   Sign in with your Google Account if prompted.

2.  **Upload the Notebook File:**
    *   Click on `File` -> `Upload notebook...`.
    *   A dialog box will appear. Click the `Choose File` button.
    *   Navigate to where you saved your assignment notebook file (e.g., `Assignment.ipynb`) on your computer and select it.
    *   Click `Open`. Colab will upload and open your notebook.

3.  **Run the Code Cells:**
    *   Now that your notebook is open and the data file is uploaded, you can run the cells sequentially.
    *   Click inside the first code cell (likely containing the imports and helper functions/classes).
    *   Press **Shift + Enter** or click the **Play button** (a circle with a triangle) to the left of the cell to execute it.
    *   Continue running each subsequent code cell one by one using **Shift + Enter** or the Play button.
    *   Run the cell corresponding to Part 1 (Breast Cancer classification). You should see its output printed below the cell.
    *   Run the cell corresponding to Part 2 (Bike Sharing regression). Make sure you have uploaded `hour.csv` *before* running this cell. You should see its output printed below.

4.  **Review the Output:**
    *   As each code cell runs, Colab displays any printed output directly below it.

You have now successfully run the assignment notebook from scratch in Google Colab!