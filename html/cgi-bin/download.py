#!/usr/bin/env python3

import os
import sys
import cgi
import cgitb
from urllib.parse import unquote
from html import escape

cgitb.enable()

current_directory = os.path.join(os.getcwd(), "html", "uploads")

# CGI 폼 데이터 가져오기
form = cgi.FieldStorage()
file_name = form.getvalue("file", "")

if file_name:
    file_name = unquote(file_name)

    # 보안 검사
    if ".." in file_name or file_name.startswith("/"):
        print("Content-type: text/html")
        print("")
        print("<!DOCTYPE html><html><body><div class='error'>")
        print(f"<p>Error: Invalid file path ({escape(file_name)}).</p>")
        print("</div></body></html>")
    else:
        full_file_path = os.path.abspath(os.path.join(current_directory, file_name))
        if not full_file_path.startswith(os.path.abspath(current_directory)):
            print("Content-type: text/html")
            print("")
            print("<!DOCTYPE html><html><body><div class='error'>")
            print(f"<p>Error: Unauthorized access ({escape(file_name)}).</p>")
            print("</div></body></html>")
        elif os.path.exists(full_file_path) and os.path.isfile(full_file_path):
            # 파일 다운로드 처리
            print("Content-Type: application/octet-stream")
            print(f"Content-Disposition: attachment; filename=\"{escape(file_name)}\"")
            print(f"Content-Length: {os.path.getsize(full_file_path)}")
            print("X-Content-Type-Options: nosniff")
            print("")
            sys.stdout.flush()
            with open(full_file_path, "rb") as file:
                sys.stdout.buffer.write(file.read())
        else:
            print("Content-type: text/html")
            print("")
            print("<!DOCTYPE html><html><body><div class='error'>")
            print(f"<p>Error: File not found ({escape(file_name)}).</p>")
            print("</div></body></html>")
