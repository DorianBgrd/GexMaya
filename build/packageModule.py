import os
import re
import argparse
import shutil
import sys


class MayaPluginModule(object):
    def __init__(self, directory, name):
        self.name = name
        self.directory = os.path.join(directory, name)
        self.module_directory = os.path.join(self.directory, "Module")

        self._descriptions = []

    def create(self):
        if not os.path.exists(self.module_directory):
            os.makedirs(self.module_directory)

        for directory in ("icons", "plug-ins", "presets",
                          "scripts", "libraries"):

            dirpath = os.path.join(self.module_directory, directory)
            if not os.path.exists(dirpath):
                os.makedirs(dirpath)

    @property
    def icons_dir(self):
        return os.path.join(self.module_directory, "icons")

    @property
    def plugins_dir(self):
        return os.path.join(self.module_directory, "plug-ins")

    @property
    def presets_dir(self):
        return os.path.join(self.module_directory, "presets")

    @property
    def scripts_dir(self):
        return os.path.join(self.module_directory, "scripts")

    @property
    def libraries_dir(self):
        return os.path.join(self.module_directory, "libraries")

    @property
    def module_description_file(self):
        return os.path.join(self.directory, f"{self.name}.mod")

    @staticmethod
    def _add_to_dir(source, directory):
        filename = os.path.basename(source)
        destination_file = os.path.join(directory, filename)
        if os.path.exists(destination_file):
            os.remove(destination_file)

        shutil.copyfile(source, destination_file)

    def add_to_icons_dir(self, path):
        self._add_to_dir(path, self.icons_dir)

    def add_to_plugins_dir(self, path):
        self._add_to_dir(path, self.plugins_dir)

    def add_to_presets_dir(self, path):
        self._add_to_dir(path, self.presets_dir)

    def add_to_scripts_dir(self, path):
        self._add_to_dir(path, self.scripts_dir)

    def add_to_libraries_dir(self, path):
        self._add_to_dir(path, self.libraries_dir)

    def copy_custom_dir(self, directory):
        dest_dir = os.path.join(self.module_directory,
                                os.path.basename(directory))

        shutil.copytree(directory, dest_dir, dirs_exist_ok=True)

    def add_custom_dir(self, dirname, path):
        custom_dir = os.path.join(self.module_directory, dirname)
        if not os.path.exists(custom_dir):
            os.makedirs(custom_dir)

        dest = os.path.join(custom_dir, os.path.basename(path))
        shutil.copyfile(path, dest)

    def add_description(self, module_name, module_version,
                        module_path, maya_version=None,
                        platform=None, env=None):
        env = env or {}

        descr = ["+"]

        if maya_version:
            descr.append(f"MAYAVERSION:{maya_version}")

        if platform:
            descr.append(f"PLATFORM:{platform}")

        descr.extend([module_name, module_version, module_path])

        line = " ".join(descr)

        for k, v in env.items():
            line += f"\n{k}:={v}"

        self._descriptions.append(line)

    def write_description_file(self):
        with open(self.module_description_file, "w") as f:
            f.write("\n".join(self._descriptions))


def create_maya_module(destination, name, icons=None, plugins=None,
                       libraries=None, presets=None, scripts=None,
                       custom_dir=None):

    package = MayaPluginModule(destination, name)

    package.create()

    for icon in icons or []:
        package.add_to_icons_dir(icon)

    for plugin in plugins or []:
        package.add_to_plugins_dir(plugin)

    for preset in presets or []:
        package.add_to_presets_dir(preset)

    for script in scripts or []:
        package.add_to_scripts_dir(script)

    for lib in libraries or []:
        package.add_to_libraries_dir(lib)

    for directory in custom_dir:
        package.copy_custom_dir(directory)

    return package


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="Package Builder",
        description="Creates a maya plugin package.",
    )

    parser.add_argument("--icons", "-ic", nargs="+", type=str)
    parser.add_argument("--presets", "-pr", nargs="+", type=str)
    parser.add_argument("--scripts", "-sc", nargs="+", type=str)
    parser.add_argument("--libraries", "-lib", nargs="+", type=str)
    parser.add_argument("--plugins", "-p", nargs="+", type=str)
    parser.add_argument("--custom", "-cs", nargs=2, type=str)
    parser.add_argument("--custom-dir", "-csd", nargs=1, type=str)
    parser.add_argument("--name", "-n", nargs=1, type=str,
                        required=True)

    parser.add_argument("--description", "-descr", nargs=1, type=str)

    parser.add_argument("directory", nargs=1)

    args = parser.parse_args(sys.argv[1:])

    module = create_maya_module(args.directory[0], args.name[0], icons=args.icons,
                                plugins=args.plugins, presets=args.presets,
                                scripts=args.scripts, custom_dir=args.custom_dir,
                                libraries=args.libraries)

    for descr in args.description:
        dscr_args = descr.split("@")

        descr_args = dscr_args[:3]

        tmp_kwargs = dscr_args[3:]

        kwargs = {}
        for elem in tmp_kwargs:
            m = re.match("(?P<k>[^=]*)=(?P<v>.*)", elem)
            if not m:
                print(f"# Warning : malformed argument {elem}.")
                continue

            key = m.group("k")
            value = m.group("v")
            if key == "env":
                value = eval(value)

            kwargs[key] = value

        if args.libraries:
            env = kwargs.setdefault("env", {})
            path = env.get("PATH", "")
            if path:
                path += ";"
            path += "libraries"
            env["PATH"] = path

        module.add_description(*descr_args, **kwargs)

    module.write_description_file()

