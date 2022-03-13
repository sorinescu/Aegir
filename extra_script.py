import gzip
import os
import pathlib
import shutil
Import("env", "projenv")

# pioPlatform = env.PioPlatform()
# print("BASE", pioPlatform.get_installed_packages())
# print("PROJ", projenv.PioPlatform().get_installed_packages())
# print(env.packages())

# .get_package_dir("framework-arduinoavr")

def applyPatches():
    print('Applying patches')
    proj_dir = env.subst("$PROJECT_DIR")
    patches_dir = os.path.join(proj_dir, 'patches')
    for item in os.listdir(patches_dir):
        if not item.endswith('.patch'):
            continue

        already_patched = os.path.join(patches_dir, f".{item}.applied")
        if os.path.exists(already_patched):
            print(f"Skipping '{item}' (already applied, because '{already_patched}' exists)")
            continue
        
        print(f"Patching from '{item}'")
        patch_file = os.path.join(patches_dir, item)
        if os.system(f"patch -d {proj_dir} -f -p0 --ignore-whitespace <{patch_file}"):
            exit(1)
        os.system(f"touch {already_patched}")


def compressFirmware(source, target, env):
    """ Compress ESP8266 firmware using gzip for 'compressed OTA upload' """
    SOURCE_FILE = env.subst("$BUILD_DIR") + os.sep + \
        env.subst("$PROGNAME") + ".bin"
    if not os.path.exists(SOURCE_FILE+'.bak'):
        print("Compressing firmware for upload...")
        shutil.move(SOURCE_FILE, SOURCE_FILE + '.bak')
        with open(SOURCE_FILE + '.bak', 'rb') as f_in:
            with gzip.open(SOURCE_FILE, 'wb') as f_out:
                shutil.copyfileobj(f_in, f_out)

    if os.path.exists(SOURCE_FILE+'.bak'):
        ORG_FIRMWARE_SIZE = os.stat(SOURCE_FILE + '.bak').st_size
        GZ_FIRMWARE_SIZE = os.stat(SOURCE_FILE).st_size

        print("Compression reduced firmware size by {:.0f}% (was {} bytes, now {} bytes)".format(
            (GZ_FIRMWARE_SIZE / ORG_FIRMWARE_SIZE) * 100, ORG_FIRMWARE_SIZE, GZ_FIRMWARE_SIZE))


def gzipResourceFiles(src):
    for item in os.listdir(src):
        s = os.path.join(src, item)
        if os.path.isdir(s):
            gzipResourceFiles(s)
        elif s.endswith(".css") or s.endswith(".js"):
            print(f"    gzipping {s}")
            with open(s, 'rb') as f_in:
                data = f_in.read()
            with gzip.open(s + '.gz', 'wb') as f_out:
                f_out.write(data)
            os.remove(s)


def buildWebDist():
    web_dir = os.path.join(env.subst("$PROJECT_DIR"), 'web')
    print(f"Running 'npm run build' in {web_dir}")
    os.system(f"cd {web_dir} && npm run build")
    src_web_dir = os.path.join(web_dir, 'data')
    dst_dir = os.path.join(env.subst("$PROJECT_DATA_DIR"), 'web')

    print(f"Cleaning {dst_dir}")
    shutil.rmtree(dst_dir, ignore_errors=True)
    pathlib.Path(dst_dir).mkdir(parents=True, exist_ok=True)

    print(f"Copying files from {src_web_dir} to {dst_dir}")
    shutil.copytree(src_web_dir, dst_dir, dirs_exist_ok=True)
    print("Gzipping resource files")
    gzipResourceFiles(dst_dir)


env.AddPreAction("upload", compressFirmware)

applyPatches()

# This doesn't work, instead we must perform the 'if' below
# env.AddPreAction("buildfs", buildWebDist)
if 'buildfs' in BUILD_TARGETS:
    buildWebDist()
