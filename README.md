# Welcome to Spresense libs for Arduino compatible project

Clone this repository.

```
$ git clone git@github.com:sonydevworld/spresense-arduino-compatible.git
```

# Directory structure

```
spresense-arduino-compatible
|-- Arduino15
|   |-- packages/SPRESENSE           - Spresense packages
|   |   |-- hardware
|   |   |   `--spresense             - Spresense reference board codes
|   |   |      `-- 1.0.0
|   |   |          |-- cores
|   |   |          |-- libraries
|   |   |          `-- variants
|   |   `-- tools                    - Necessary tool chain and prebuilt binaries
|   |       |-- spresense-tools      - Spresense tools
|   |       |   `-- 1.0.0
|   |       |-- spresense-sdk        - Spresense SDK prebuilt binaries
|   |       |   `-- 1.0.0
|   |       `-- gcc-arm-none-eabi    - GCC compiler
|   |           `-- 5.4.1
|   `-- package_spresense_index.json - Arduino IDE configuration json file
`-- tools                            - Import/Export tools
```

# Getting started
### [Spresense Arduino Library Getting Started Guide (English)](https://developer.sony.com/develop/spresense/docs/arduino_set_up_en.html)

### [Spresense Arduino スタートガイド (日本語)](https://developer.sony.com/develop/spresense/docs/arduino_set_up_ja.html)

# How to prepare Arduino environment
## Pull or Import GCC and Prebuilt SDK

```
./tools/prepare_arduino.sh [OPTIONS...]
```

### Options

#### For using local archive

| Option | Argument                          | Note                                           |
|-------:|:----------------------------------|:-----------------------------------------------|
| -g     | path/to/GCC-archive-path          | GCC archive path                               |
| -s     | path/to/SDK-archive-path          | Prebuilt SDK archive path                      |

#### For using local source code

| Option | Argument                          | Note                                           |
|-------:|:----------------------------------|:-----------------------------------------------|
| -S     | path/to/spresense-sdk-path        | Local Spresense SDK build root path            |
| -v     | Board_variant                     | Target board variant (default:spresense)       |
| -c     | spresense or spresense_sub        | Target SDK configuration (default: spresense)  |
| -k     | release or debug                  | Target kernel configuration (default: release) |
| -M     | "SDK" or "Kernel" or "SDK/Kernel" | Manual configuration by menuconfig             |
| -G     | "SDK" or "Kernel" or "SDK/Kernel" | Manual configuration by gconfig                |
| -Q     | "SDK" or "Kernel" or "SDK/Kernel" | Manual configuration by qconfig                |
| -i     | -                                 | Do not change Kernel/SDK configuration         |

#### Other option

| Option | Argument                          | Note                                           |
|-------:|:----------------------------------|:-----------------------------------------------|
| -H     | Windows or Linux64 or Mac         | Arduino IDE Host OS                            |
| -p     | -                                 | No network access option                       |
| -h     | -                                 | Show help                                      |

### Example

#### Export Spresense SDK prebuilt binary for MainCore/Debug:Disabled from local repository.

```
$ ./tools/prepare_arduino.sh -S path/to/spresense -p
```

#### Export Spresense SDK prebuilt binary for MainCore/Debug:Enabled from local repository.

```
$ ./tools/prepare_arduino.sh -S path/to/spresense -k debug -p
```

#### Export Spresense SDK prebuilt binary for SubCore/Debug:Disabled from local repository.

```
$ ./tools/prepare_arduino.sh -S path/to/spresense -c spresense_sub -k subcore-release -p
```

#### Export Spresense SDK prebuilt binary for SubCore/Debug:Enabled from local repository.

```
$ ./tools/prepare_arduino.sh -S path/to/spresense -c spresense_sub -k subcore-debug -p
```

#### Export Spresense SDK prebuilt binary for MainCore/Debug:Disabled from local repository *with configuration change*.

```
$ ./tools/prepare_arduino.sh -S path/to/spresense -M SDK/Kernel -p
```

* Menu configuration will open twice as `NuttX Configuration` and `SDK configuration`

# Creating Arduino boards manager local install package

To create a Arduino boards manager files for manual installation simply type:

```
$ make
```

This command will generate the following files:

```
out/package_spresense_local_index.json
out/staging/spresense-v*.*.*_local.tar.gz
out/staging/spresense-tools-v*.*.*_local.tar.gz
out/staging/spresense-sdk-v*.*.*_local.tar.gz
```

# Manual install

1. Copy `out/staging` into *\<Arduino15\>* directory.
2. Set *package_spresense_local_index.json URL* into `Additional Boards Manager URLs` in Arduino IDE.
3. Install local spresense board package by `Boards Manager` in Arduino IDE as same as official package. 

## *\<Arduino15\>*:
* Windows: `%userprofile%\AppData\Local\Arduino15`
* macOS: `~/Library/Arduino15`
* Ubuntu: `~/.arduino15`

## *package_spresense_local_index.json URL*:
`file://path/to/out/package_spresense_local_index.json`