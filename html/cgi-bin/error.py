#!/usr/bin/env python3

import os
import cgitb

# Enable debugging
cgitb.enable()

print("Content-type: text/html\n")
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>Infinite Loop Error</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }")
print("        h1 { color: #990000; }")
print("        .error { background-color: #f8d7da; color: #721c24; padding: 15px; border-radius: 4px; margin-bottom: 20px; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <h1>Error Simulation</h1>")

# 무한 루프 발생
print("    <div class='error'>")
print("        <p>Warning: This script contains an infinite loop!</p>")
print("    </div>")

while True:
    pass  # 무한 루프

print("</body>")
print("</html>")
