import os

### CONSTANT VARIABLES ###
FITX_ROOT     = os.environ.get("FITX_ROOT", "/FiTx")
LINUX_ROOT    = os.environ.get("LINUX_ROOT", "/linux")
LOG_DIR       = os.path.abspath("/tmp/log")
BUILD_DIR     = os.path.join(FITX_ROOT, 'build')
DETECTOR_PATH = os.path.join(BUILD_DIR, 'detector', 'all_detector',
                             'libAllDetectorMod.so')
