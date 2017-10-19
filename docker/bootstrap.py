import os
import shutil
from configparser import ConfigParser

COOLQ_DIR = '/home/user/coolq'
COOLQ_CONFIG_FILE = os.path.join(COOLQ_DIR, 'conf', 'CQP.cfg')
COOLQ_APP_DIR = os.path.join(COOLQ_DIR, 'app')
APP_ID = 'io.github.richardchien.coolqhttpapi'
APP_DIR = os.path.join(COOLQ_APP_DIR, APP_ID)
APP_LOCK_FILE = os.path.join(APP_DIR, 'app.lock')
APP_CONFIG_FILE = os.path.join(APP_DIR, 'config.cfg')
APP_CONFIG_ENV_PREFIX = 'CQHTTP_'

TMP_CPK_FILE = '/tmp/io.github.richardchien.coolqhttpapi.cpk'


def touch(path):
    with open(path, 'a'):
        os.utime(path, None)


def is_first_start():
    return not os.path.exists(APP_DIR)


def init():
    os.makedirs(APP_DIR, exist_ok=True)
    touch(APP_LOCK_FILE)


def is_locked():
    return os.path.exists(APP_LOCK_FILE)


def bootstrap():
    if is_first_start():
        init()

    if is_locked():
        # we can do whatever we want here

        # copy cpk
        shutil.copyfile(
            TMP_CPK_FILE,
            os.path.join(COOLQ_APP_DIR, APP_ID + '.cpk')
        )

        # enable plugin
        coolq_config = ConfigParser()
        coolq_config.read(COOLQ_CONFIG_FILE)
        if 'App' in coolq_config:
            coolq_config['App'][APP_ID + '.status'] = '1'
        else:
            coolq_config['App'] = {
                APP_ID + '.status': '1'
            }
        with open(COOLQ_CONFIG_FILE, 'w') as config_file:
            coolq_config.write(config_file)

        # write app config
        app_config = ConfigParser()
        app_config['general'] = {}
        for key, value in os.environ.items():
            if not key.startswith(APP_CONFIG_ENV_PREFIX):
                continue

            key = key[len(APP_CONFIG_ENV_PREFIX):]
            app_config['general'][key.lower()] = value
        with open(APP_CONFIG_FILE, 'w') as config_file:
            app_config.write(config_file)


if __name__ == '__main__':
    bootstrap()
