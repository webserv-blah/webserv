#!/usr/bin/env python3

import os
import cgi
import cgitb
import shutil

# Enable debugging
cgitb.enable()

UPLOAD_DIR = "html/uploads"  # 업로드된 파일을 저장할 디렉토리
os.makedirs(UPLOAD_DIR, exist_ok=True)

print("Content-type: text/html\n")
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>File Upload</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }")
print("        h1 { color: #336699; }")
print("        form { background: #f9f9f9; padding: 20px; border-radius: 5px; max-width: 500px; }")
print("        label { display: block; margin-bottom: 5px; font-weight: bold; }")
print("        input[type='file'] { margin-bottom: 15px; }")
print("        input[type='submit'] { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }")
print("        input[type='submit']:hover { background-color: #45a049; }")
print("        .success { background-color: #dff0d8; color: #3c763d; padding: 15px; border-radius: 4px; margin-bottom: 20px; }")
print("        .error { background-color: #f8d7da; color: #721c24; padding: 15px; border-radius: 4px; margin-bottom: 20px; }")
print("        .data-display { background: #f4f4f4; padding: 15px; border-radius: 5px; margin-top: 20px; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <h1>File Upload Example</h1>")

# Handle file upload from form
form = cgi.FieldStorage()
if os.environ.get('REQUEST_METHOD', '') == 'POST':
    file_item = form['file']
    if file_item.filename:
        file_path = os.path.join(UPLOAD_DIR, os.path.basename(file_item.filename))
        with open(file_path, 'wb') as f:
            f.write(file_item.file.read())
        
        print("    <div class='success'>")
        print("        <p>File successfully uploaded!</p>")
        print("    </div>")
        
        print("    <div class='data-display'>")
        print("        <h2>Uploaded File Information:</h2>")
        print(f"        <p><strong>Filename:</strong> {file_item.filename}</p>")
        print(f"        <p><strong>Saved to:</strong> {file_path}</p>")
        print("    </div>")
    else:
        print("    <div class='error'>")
        print("        <p>No file was uploaded.</p>")
        print("    </div>")

# Handle file upload from PATH_INFO
path_info = os.environ.get('PATH_INFO', '')
if path_info:
    file_abs_path = os.path.abspath(path_info.strip('/'))
    if os.path.exists(file_abs_path) and os.path.isfile(file_abs_path):
        dest_path = os.path.join(UPLOAD_DIR, os.path.basename(file_abs_path))
        shutil.copy(file_abs_path, dest_path)
        
        print("    <div class='success'>")
        print("        <p>File successfully copied from PATH_INFO!</p>")
        print("    </div>")
        
        print("    <div class='data-display'>")
        print("        <h2>Copied File Information:</h2>")
        print(f"        <p><strong>Source Path:</strong> {file_abs_path}</p>")
        print(f"        <p><strong>Saved to:</strong> {dest_path}</p>")
        print("    </div>")
    else:
        print("    <div class='error'>")
        print("        <p>Invalid file path or file does not exist.</p>")
        print("    </div>")

# Always display the form
print("    <form method='post' action='upload.py' enctype='multipart/form-data'>")
print("        <div>")
print("            <label for='file'>Choose a file to upload:</label>")
print("            <input type='file' id='file' name='file' required>")
print("        </div>")
print("        <div>")
print("            <input type='submit' value='Upload'>")
print("        </div>")
print("    </form>")

print("</body>")
print("</html>")