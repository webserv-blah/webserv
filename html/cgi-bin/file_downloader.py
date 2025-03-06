#!/usr/bin/env python3

import os
import sys
import cgi
import cgitb
from urllib.parse import unquote
from html import escape

cgitb.enable()

current_directory = os.path.join(os.getcwd(), "html", "uploads")

# URL 경로에서 파일 이름 얻기
path_info = os.environ.get("PATH_INFO", "")
file_name = unquote(path_info.lstrip('/'))

# HTML 페이지 출력 함수
def print_html_page(selected_file, files):
    print("Content-type: text/html")
    print("")
    print("<!DOCTYPE html>")
    print("<html><head><title>File Downloader</title>")
    print("<style>")
    print("body { font-family: Arial, sans-serif; text-align: center; margin: 50px; background-color: #f4f4f4; }")
    print("h1 { color: #336699; }")
    print(".container { max-width: 700px; margin: auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }")
    print(".file-list { text-align: left; padding: 10px; background: #eef3f7; border-radius: 5px; }")
    print(".file-list ul { list-style: none; padding: 0; }")
    print(".file-list li { padding: 10px; border-bottom: 1px solid #ddd; }")
    print(".file-list a { text-decoration: none; color: #336699; font-weight: bold; }")
    print(".file-list a:hover { text-decoration: underline; }")
    print(".argv-box { background: #fafafa; padding: 15px; border-left: 4px solid #336699; margin: 10px 0; border-radius: 5px; text-align: left; }")
    print("pre { background: #333; color: #f8f8f8; padding: 10px; border-radius: 5px; text-align: left; overflow-x: auto; }")
    print("</style></head><body><div class='container'>")
    print("<h1>File Downloader</h1><p>Select a file to download:</p>")
    print("<div class='file-list'><ul>")
    if files:
        for f in files:
            print(f"<li><a href='/cgi-bin/file_downloader.py/{escape(f)}'>{escape(f)}</a></li>")
    else:
        print("<li>No files found.</li>")
    print("</ul></div>")
    
    # argv 출력 추가
    print("<h2>Command-line Arguments (argv):</h2>")
    print("<div class='argv-box'><pre>")
    for i, arg in enumerate(sys.argv):
        print(f"argv[{i}]: {escape(arg)}")
    print("</pre></div>")
    
    # 선택한 파일이 있으면 다운로드 버튼 제공
    if selected_file:
        print("<h2>Selected File:</h2>")
        print(f"<p>{escape(selected_file)}</p>")
        print(f"<form action='/cgi-bin/download.py' method='get'>")
        print(f"<input type='hidden' name='file' value='{escape(selected_file)}'>")
        print(f"<input type='submit' value='Download {escape(selected_file)}'>")
        print("</form>")
    
    print("</div></body></html>")

# 파일 목록 가져오기
try:
    files = [f for f in os.listdir(current_directory) if os.path.isfile(os.path.join(current_directory, f))]
except FileNotFoundError:
    files = []

# path_info를 통해 전달된 파일이 있는 경우, argv 출력 후 다운로드 버튼 표시
print_html_page(file_name, files)
