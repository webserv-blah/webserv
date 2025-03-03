#!/Users/damin/francinette/venv/bin/python3

import os
import cgi
import cgitb
from html import escape

# Enable debugging
cgitb.enable()

print("Content-type: text/html\n")
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>Python Form Processor</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }")
print("        h1 { color: #336699; }")
print("        form { background: #f9f9f9; padding: 20px; border-radius: 5px; max-width: 500px; }")
print("        label { display: block; margin-bottom: 5px; font-weight: bold; }")
print("        input[type='text'], input[type='email'], textarea { width: 100%; padding: 8px; margin-bottom: 15px; border: 1px solid #ddd; border-radius: 4px; }")
print("        input[type='submit'] { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }")
print("        input[type='submit']:hover { background-color: #45a049; }")
print("        .success { background-color: #dff0d8; color: #3c763d; padding: 15px; border-radius: 4px; margin-bottom: 20px; }")
print("        .data-display { background: #f4f4f4; padding: 15px; border-radius: 5px; margin-top: 20px; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <h1>Python Form Example</h1>")

# Determine if this is a form submission
form = cgi.FieldStorage()
if os.environ.get('REQUEST_METHOD', '') == 'POST':
    name = escape(form.getvalue('name', ''))
    email = escape(form.getvalue('email', ''))
    message = escape(form.getvalue('message', ''))
    
    print("    <div class='success'>")
    print("        <p>Thank you for your submission!</p>")
    print("    </div>")
    
    print("    <div class='data-display'>")
    print("        <h2>Form Data Received:</h2>")
    print(f"        <p><strong>Name:</strong> {name}</p>")
    print(f"        <p><strong>Email:</strong> {email}</p>")
    print(f"        <p><strong>Message:</strong> {message}</p>")
    print("    </div>")

# Always display the form
print("    <form method='post' action='form.py'>")
print("        <div>")
print("            <label for='name'>Name:</label>")
print("            <input type='text' id='name' name='name' required>")
print("        </div>")
print("        <div>")
print("            <label for='email'>Email:</label>")
print("            <input type='email' id='email' name='email' required>")
print("        </div>")
print("        <div>")
print("            <label for='message'>Message:</label>")
print("            <textarea id='message' name='message' rows='5' required></textarea>")
print("        </div>")
print("        <div>")
print("            <input type='submit' value='Submit'>")
print("        </div>")
print("    </form>")

print("</body>")
print("</html>")
