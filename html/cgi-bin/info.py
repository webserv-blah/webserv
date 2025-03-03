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
print("    <title>Python CGI Example</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }")
print("        h1 { color: #336699; }")
print("        h2 { color: #444; margin-top: 20px; }")
print("        pre { background: #f4f4f4; padding: 10px; border-radius: 5px; }")
print("        .data-section { margin-bottom: 30px; }")
print("        table { border-collapse: collapse; width: 100%; }")
print("        table, th, td { border: 1px solid #ddd; }")
print("        th, td { padding: 8px; text-align: left; }")
print("        th { background-color: #f2f2f2; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <h1>Python CGI Example</h1>")

# Display CGI environment variables
print("    <div class='data-section'>")
print("        <h2>CGI Environment Variables</h2>")
print("        <table>")
print("            <tr><th>Variable</th><th>Value</th></tr>")

env_vars = [
    'GATEWAY_INTERFACE', 'SERVER_PROTOCOL', 'SERVER_SOFTWARE', 'SERVER_NAME',
    'SERVER_PORT', 'REQUEST_METHOD', 'PATH_INFO', 'SCRIPT_NAME', 'QUERY_STRING',
    'REMOTE_ADDR', 'CONTENT_TYPE', 'CONTENT_LENGTH'
]

for var in env_vars:
    if var in os.environ:
        print(f"            <tr><td>{escape(var)}</td><td>{escape(os.environ[var])}</td></tr>")

print("        </table>")
print("    </div>")

# Process and display GET and POST parameters
form = cgi.FieldStorage()

def display_params(title, params):
    if params:
        print(f"    <div class='data-section'>")
        print(f"        <h2>{title}</h2>")
        print("        <table>")
        print("            <tr><th>Parameter</th><th>Value</th></tr>")
        for key in params:
            print(f"            <tr><td>{escape(key)}</td><td>{escape(params.getvalue(key))}</td></tr>")
        print("        </table>")
        print("    </div>")

display_params("GET and POST Parameters", form)

# Display request headers
print("    <div class='data-section'>")
print("        <h2>HTTP Request Headers</h2>")
print("        <table>")
print("            <tr><th>Header</th><th>Value</th></tr>")

for header, value in os.environ.items():
    if header.startswith("HTTP_"):
        header_name = header[5:].replace("_", "-").title()
        print(f"            <tr><td>{escape(header_name)}</td><td>{escape(value)}</td></tr>")

print("        </table>")
print("    </div>")

print("</body>")
print("</html>")
