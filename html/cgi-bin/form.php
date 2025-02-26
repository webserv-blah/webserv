#!/usr/bin/php-cgi
<?php
// Set content type to HTML
header("Content-type: text/html");

// Determine if this is a form submission or initial page load
$is_submission = ($_SERVER['REQUEST_METHOD'] === 'POST');

// Start HTML document
echo "<!DOCTYPE html>\n";
echo "<html>\n";
echo "<head>\n";
echo "    <title>PHP Form Processor</title>\n";
echo "    <style>\n";
echo "        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }\n";
echo "        h1 { color: #336699; }\n";
echo "        form { background: #f9f9f9; padding: 20px; border-radius: 5px; max-width: 500px; }\n";
echo "        label { display: block; margin-bottom: 5px; font-weight: bold; }\n";
echo "        input[type='text'], input[type='email'], textarea { width: 100%; padding: 8px; margin-bottom: 15px; border: 1px solid #ddd; border-radius: 4px; }\n";
echo "        input[type='submit'] { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }\n";
echo "        input[type='submit']:hover { background-color: #45a049; }\n";
echo "        .success { background-color: #dff0d8; color: #3c763d; padding: 15px; border-radius: 4px; margin-bottom: 20px; }\n";
echo "        .data-display { background: #f4f4f4; padding: 15px; border-radius: 5px; margin-top: 20px; }\n";
echo "    </style>\n";
echo "</head>\n";
echo "<body>\n";
echo "    <h1>PHP Form Example</h1>\n";

// If the form was submitted, process the data
if ($is_submission) {
    $name = isset($_POST['name']) ? htmlspecialchars($_POST['name']) : '';
    $email = isset($_POST['email']) ? htmlspecialchars($_POST['email']) : '';
    $message = isset($_POST['message']) ? htmlspecialchars($_POST['message']) : '';
    
    echo "    <div class='success'>\n";
    echo "        <p>Thank you for your submission!</p>\n";
    echo "    </div>\n";
    
    echo "    <div class='data-display'>\n";
    echo "        <h2>Form Data Received:</h2>\n";
    echo "        <p><strong>Name:</strong> $name</p>\n";
    echo "        <p><strong>Email:</strong> $email</p>\n";
    echo "        <p><strong>Message:</strong> $message</p>\n";
    echo "    </div>\n";
}

// Always display the form (whether it's initial load or after submission)
echo "    <form method='post' action='form.php'>\n";
echo "        <div>\n";
echo "            <label for='name'>Name:</label>\n";
echo "            <input type='text' id='name' name='name' required>\n";
echo "        </div>\n";
echo "        <div>\n";
echo "            <label for='email'>Email:</label>\n";
echo "            <input type='email' id='email' name='email' required>\n";
echo "        </div>\n";
echo "        <div>\n";
echo "            <label for='message'>Message:</label>\n";
echo "            <textarea id='message' name='message' rows='5' required></textarea>\n";
echo "        </div>\n";
echo "        <div>\n";
echo "            <input type='submit' value='Submit'>\n";
echo "        </div>\n";
echo "    </form>\n";

// End HTML document
echo "</body>\n";
echo "</html>\n";
?>