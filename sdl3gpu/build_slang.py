import argparse
from pathlib import Path
import subprocess
import shutil


def delete_spv(path):
    print('Deleting spv from ' + str(path))

    for spv in path.glob('*.spv'):
        spv.unlink(missing_ok=True)


def build_slang(shader_path, build_type):
    print('Building Slang...')

    skip_files = ['utils.slang']

    for slang in shader_path.glob('*.slang'):
        for skip in skip_files:
            if not slang.match(skip):
                spv_name = str(slang) + '.spv'

                cmd = 'slangc ' + str(slang)
                if (build_type == 'Debug'):
                    cmd += ' -g3 -O0'
                elif (build_type == 'MinSizeRel'):
                    cmd += ' -g0 -O3'

                cmd += ' -matrix-layout-column-major -o ' + str(spv_name)

                subprocess.call(cmd)


def copy_spv(src, dst):
    print('Copy spv to ' + str(dst))

    if not dst.exists():
        dst.mkdir(parents=True, exist_ok=True)

    for src_spv in src.glob('*.spv'):
        shutil.copy(src_spv, dst)


def main(args):
    local_shader_path = Path('.').resolve().parent / 'shaders' / 'slang'
    delete_spv(local_shader_path)
    build_slang(local_shader_path, args.build_type)

    remote_shader_path = Path(args.output_dir) / 'shaders' / 'slang'
    delete_spv(remote_shader_path)
    copy_spv(local_shader_path, remote_shader_path)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('source_dir')
    parser.add_argument('build_type')
    parser.add_argument('output_dir')

    main(parser.parse_args())
