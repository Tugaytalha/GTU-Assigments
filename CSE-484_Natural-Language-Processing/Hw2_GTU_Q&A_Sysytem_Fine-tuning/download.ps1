# Create a directory to store downloaded files
$downloadDirectory = "C:\GTU_Downloads"
if (!(Test-Path -Path $downloadDirectory)) {
  New-Item -ItemType Directory -Path $downloadDirectory
}

# URL of the webpage
$url = "https://www.gtu.edu.tr/kategori/3075/0/display.aspx?languageId=1"

# Fetch the webpage content
$response = Invoke-WebRequest -Uri $url -UseBasicParsing

# Find all links to PDF files
$pdfLinks = $response.Links | Where-Object {$_.href -like "*.pdf"}

# Filter out English files based on filename (case-insensitive)
$filteredPdfLinks = $pdfLinks | Where-Object {
  $_.href -notmatch "(?i)(Directive|Regulation|Education|Principles|Guidelines|Programs|Courses|Internship|Establishment|Operation|Practices|Implementation|Guide|Guidebook|Handbook|Manual)"
}

# Download each PDF file
foreach ($link in $filteredPdfLinks) {
  $fileUrl = $link.href
  
  # Construct the full URL if it's a relative path
  if (!($fileUrl.StartsWith("http"))) {
    $fileUrl = "https://www.gtu.edu.tr" + $fileUrl
  }

  # Extract the filename from the URL
  $fileName = Split-Path -Path $fileUrl -Leaf

  # Download the file using Invoke-WebRequest
  try {
    Invoke-WebRequest -Uri $fileUrl -OutFile "$downloadDirectory\$fileName"
    Write-Host "Downloaded: $fileName"
  }
  catch {
    Write-Host "Error downloading: $fileName"
    Write-Host $_
  }
}

Write-Host "Download process completed."