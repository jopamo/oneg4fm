import os
import glob
import subprocess

libfm_qt_src = 'libfm-qt/src'
search_dirs = ['pcmanfm', 'src']
libfm_internal = 'libfm-qt/src'

headers = glob.glob(os.path.join(libfm_qt_src, '*.h'))
headers = [os.path.basename(h) for h in headers]

unused_external = []
unused_internal = []

print("Analyzing usage of libfm-qt headers...")

for h in headers:
    # Check external usage
    cmd = ['grep', '-r', h] + search_dirs
    result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    if h == 'filesearchdialog.h':
        print(f"DEBUG: {h} return code {result.returncode}")

    if result.returncode != 0:
        unused_external.append(h)
        
        # Check internal usage (if not used externally)
        # Exclude the file itself from grep
        cmd_internal = f"grep -r {h} {libfm_internal} --exclude={h}"
        result_internal = subprocess.run(cmd_internal, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
        if result_internal.returncode != 0:
            unused_internal.append(h)

print(f"\nTotal headers: {len(headers)}")
print(f"Headers NOT used in pcmanfm/ or src/: {len(unused_external)}")
for h in unused_external:
    print(f"  - {h}")

print(f"\nHeaders NOT used ANYWHERE (candidates for removal): {len(unused_internal)}")
for h in unused_internal:
    print(f"  - {h}")
