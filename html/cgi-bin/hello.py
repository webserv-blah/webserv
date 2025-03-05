#!/usr/bin/env python3

import os
import cgi
import cgitb
from datetime import datetime
from html import escape

# Enable debugging
cgitb.enable()

print("Content-type: text/html\n")
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>Hello from Python!</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }")
print("        h1 { color: #336699; }")
print("        p { color: #444; }")
print("        .container { max-width: 600px; margin: 0 auto; padding: 20px; border: 1px solid #ddd; border-radius: 5px; }")
print("        .time { font-size: 0.9em; color: #777; margin-top: 30px; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")

# Get the name parameter from the query string or use a default
form = cgi.FieldStorage()
name = escape(form.getvalue("name", "World"))

print(f"        <h1>Hello, {name}!</h1>")
print("        <p>This is a simple Python CGI script running on the custom webserver.</p>")
print("        <p>Try adding a name parameter to the URL: <code>?name=YourName</code></p>")
print(f"        <p class='time'>Current server time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>")
print("    </div>")
print("</body>")
print("</html>")
