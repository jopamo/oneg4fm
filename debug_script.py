
import os
import glob
import subprocess

libfm_qt_src = 'libfm-qt/src'
search_dirs = ['pcmanfm', 'src']
libfm_internal = 'libfm-qt/src'

headers = glob.glob(os.path.join(libfm_qt_src, '*.h'))
headers = [os.path.basename(h) for h in headers]

# Sort for deterministic output
headers.sort()

for h in headers:
    if h == 'filesearchdialog.h':
        cmd = ['grep', '-r', h] + search_dirs
        print(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(f"Return code: {result.returncode}")
        print(f"Stdout: {result.stdout.decode()}")
        print(f"Stderr: {result.stderr.decode()}")
