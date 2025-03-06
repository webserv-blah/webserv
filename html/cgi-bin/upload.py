#!/usr/bin/env python3

import os
import cgi
import cgitb
import shutil
from urllib.parse import unquote
from html import escape

cgitb.enable()

UPLOAD_DIR = os.path.join(os.getcwd(), "html", "uploads")
os.makedirs(UPLOAD_DIR, exist_ok=True)

print("Content-type: text/html\n")

print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>File Upload Status</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }")
print("        .success { background-color: #dff0d8; color: #3c763d; padding: 15px; border-radius: 4px; margin-bottom: 20px; }")
print("        .error { background-color: #f8d7da; color: #721c24; padding: 15px; border-radius: 4px; margin-bottom: 20px; }")
print("        .data-display { background: #f4f4f4; padding: 15px; border-radius: 5px; margin-top: 20px; }")
print("    </style>")
print("</head>")
print("<body>")

try:
    # === `PATH_INFO`에서 파일 경로 가져오기 ===
    path_info = os.environ.get("PATH_INFO", "").lstrip('/')
    file_abs_path = unquote(path_info)

    if file_abs_path:
        # 보안 검사: 상대 경로나 잘못된 경로 방지
        if ".." in file_abs_path or not os.path.isfile(file_abs_path):
            print("<div class='error'><p>Error: Invalid or unauthorized file path.</p></div>")
        else:
            dest_path = os.path.join(UPLOAD_DIR, os.path.basename(file_abs_path))
            shutil.copy(file_abs_path, dest_path)

            print("<div class='success'><p>File successfully uploaded!</p></div>")
            print("<div class='data-display'>")
            print(f"    <p><strong>Source Path:</strong> {escape(file_abs_path)}</p>")
            print(f"    <p><strong>Saved to:</strong> {escape(dest_path)}</p>")
            print("</div>")

    else:
        print("<div class='error'><p>Error: No file specified.</p></div>")

except Exception as e:
    print("<div class='error'><p>Error: Internal Server Error</p></div>")
    print(f"<pre>{escape(str(e))}</pre>")

print("</body>")
print("</html>")
