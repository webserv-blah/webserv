#!/usr/bin/env python3

import os
import cgi
import cgitb
import mimetypes
import urllib.parse
import sys
from datetime import datetime
from html import escape

# Enable debugging
cgitb.enable()

print("Content-type: text/html\n")

# 기존 작업 디렉터리에 /uploads 추가
current_directory = os.path.join(os.getcwd(), "html", "uploads")

print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>File Viewer</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; text-align: center; margin: 50px; background-color: #f4f4f4; }")
print("        h1 { color: #336699; }")
print("        .container { max-width: 700px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); }")
print("        .file-list { text-align: left; margin-top: 20px; padding: 10px; border-radius: 5px; background: #eef3f7; }")
print("        .file-list ul { list-style: none; padding: 0; }")
print("        .file-list li { padding: 10px; border-bottom: 1px solid #ddd; }")
print("        .file-list a { text-decoration: none; color: #336699; font-weight: bold; }")
print("        .file-list a:hover { text-decoration: underline; }")
print("        .file-box { background: #fafafa; padding: 15px; border-left: 4px solid #336699; margin: 10px 0; border-radius: 5px; text-align: left; }")
print("        .download-btn { display: inline-block; padding: 10px 15px; margin-top: 10px; color: white; background: #ff6600; text-decoration: none; border-radius: 5px; font-weight: bold; }")
print("        .download-btn:hover { background: #cc5500; }")
print("        pre { background: #333; color: #f8f8f8; padding: 10px; border-radius: 5px; text-align: left; overflow-x: auto; }")
print("        .time { font-size: 0.9em; color: #777; margin-top: 20px; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")
print("        <h1>File Viewer</h1>")
print("        <p>Select files to open:</p>")

# 파일 목록 가져오기
try:
    files = os.listdir(current_directory)
    files = [f for f in files if os.path.isfile(os.path.join(current_directory, f))]
except FileNotFoundError:
    files = []

# 파일 선택 폼 출력
print("        <form method='get' action='file_reader.py'>")
print("            <select name='file' multiple>")  # 다중 선택 가능
for f in files:
    print(f"                <option value='{escape(f)}'>{escape(f)}</option>")
print("            </select>")
print("            <input type='submit' value='Open'>")
print("        </form>")

# 파일 목록 출력
print("        <div class='file-list'>")
print("            <h2>Available Files:</h2>")
if files:
    print("            <ul>")
    for f in files:
        print(f"                <li><a href='file_reader.py?file={escape(f)}'>{escape(f)}</a></li>")
    print("            </ul>")
else:
    print("            <p>No files found.</p>")
print("        </div>")

# CGI 폼 데이터 가져오기
form = cgi.FieldStorage()
file_paths = form.getlist("file")  # 여러 개의 파일 받기

# 선택한 파일 표시
if file_paths:
    print("        <h2>Selected Files:</h2>")
    
    for file_path in file_paths:
        if ".." in file_path or file_path.startswith("/"):
            print(f"<p>Error: Invalid file path ({escape(file_path)}).</p>")
            continue

        full_file_path = os.path.abspath(os.path.join(current_directory, file_path))

        try:
            mime_type, _ = mimetypes.guess_type(full_file_path)
            file_url = f"/uploads/{urllib.parse.quote(file_path)}"

            print(f"        <div class='file-box'>")
            print(f"        <h3>{escape(file_path)}</h3>")

            if mime_type:
                if mime_type.startswith("text"):  # 텍스트 파일
                    with open(full_file_path, "r", encoding="utf-8") as file:
                        print("<pre>")
                        print(escape(file.read()))
                        print("</pre>")
                elif mime_type.startswith("image"):  # 이미지 파일
                    print(f"        <img src='{file_url}' alt='{escape(file_path)}' style='max-width:500px;'><br>")
                elif mime_type == "application/pdf":  # PDF 파일
                    print(f"        <iframe src='{file_url}' width='80%' height='600px'></iframe>")
                elif mime_type.startswith("video"):  # 동영상 파일
                    print(f"        <video controls width='80%'><source src='{file_url}' type='{mime_type}'>Your browser does not support the video tag.</video>")
                elif mime_type.startswith("audio"):  # 오디오 파일
                    print(f"        <audio controls><source src='{file_url}' type='{mime_type}'>Your browser does not support the audio tag.</audio>")
                else:  # 다운로드 가능 파일
                    print(f"        <a class='download-btn' href='{file_url}' download>Download {escape(file_path)}</a>")
            else:
                print(f"        <a class='download-btn' href='{file_url}' download>Download {escape(file_path)}</a>")

            print("        </div>")

        except FileNotFoundError:
            print(f"<p>Error: File not found ({escape(file_path)}).</p>")
        except Exception as e:
            print(f"<p>Error: {escape(str(e))}</p>")

# 현재 서버 시간 출력
print(f"        <p class='time'>Current server time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>")
print("    </div>")
print("</body>")
print("</html>")
