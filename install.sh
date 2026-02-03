#!/bin/bash

readonly RELEASES_URL="https://api.github.com/repos/gallardo994/xx-cli/releases"
readonly INSTALL_DIRECTORY="/usr/local/bin"
readonly BINARY_NAME="xx"
readonly DOWNLOAD_URL_TEMPLATE="https://github.com/Gallardo994/xx-cli/releases/download/%s/xx-build_%s_release"

readonly LLVM_COMPILER_NAME="llvm_20"

get_latest_release() {
    curl -sSL "${RELEASES_URL}/latest" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/'
}

install_xx_cli() {
    local LATEST_RELEASE
    LATEST_RELEASE=$(get_latest_release)

    if [ -z "${LATEST_RELEASE}" ]; then
        echo "Failed to fetch the latest release version. Cancelling." >&2
        exit 1
    fi

    local PLATFORM=$(uname | tr '[:upper:]' '[:lower:]')
    local ARCH=$(uname -m)

    case "${ARCH}" in
        x86_64) ARCH="x86_64" ;;
        aarch64 | arm64) ARCH="arm64" ;;
        *)
            echo "Unsupported architecture: ${ARCH}. Cancelling." >&2
            exit 1
            ;;
    esac

    case "${PLATFORM}" in
        darwin) PLATFORM="darwin" ;;
        linux) PLATFORM="linux" ;;
        *)
            echo "Unsupported platform: ${PLATFORM}. Cancelling." >&2
            exit 1
            ;;
    esac

    local DOWNLOAD_TARGET=""

    if [ "${PLATFORM}" = "darwin" ]; then
        if [ "${ARCH}" = "arm64" ]; then
            DOWNLOAD_TARGET="macos_latest_${LLVM_COMPILER_NAME}"
        else
            echo "Unsupported architecture for macOS: ${ARCH}. Cancelling." >&2
            exit 1
        fi
    fi

    if [ "${PLATFORM}" = "linux" ]; then
        if [ "${ARCH}" = "x86_64" ]; then
            DOWNLOAD_TARGET="ubuntu_latest_${LLVM_COMPILER_NAME}"
        elif [ "${arch}" = "arm64" ]; then
            DOWNLOAD_TARGET="ubuntu_24.04_arm_${LLVM_COMPILER_NAME}"
        else
            echo "Unsupported architecture for Linux: ${ARCH}. Cancelling." >&2
            exit 1
        fi
    fi

    if [ -z "${DOWNLOAD_TARGET}" ]; then
        echo "Unsupported platform: ${PLATFORM} ${ARCH}. Cancelling." >&2
        exit 1
    fi

    DOWNLOAD_URL=$(printf "${DOWNLOAD_URL_TEMPLATE}" "${LATEST_RELEASE}" "${DOWNLOAD_TARGET}")
    DOWNLOAD_TEMP_FILE="$(mktemp)"

    printf "Downloading ${DOWNLOAD_TARGET} from %s\n" "${DOWNLOAD_URL}"
    curl -L -o "${DOWNLOAD_TEMP_FILE}" "${DOWNLOAD_URL}"
    chmod +x "${DOWNLOAD_TEMP_FILE}"

    printf "Installing xx-cli to %s\n" "${INSTALL_DIRECTORY}/${BINARY_NAME}"
    sudo mv "${DOWNLOAD_TEMP_FILE}" "${INSTALL_DIRECTORY}/${BINARY_NAME}"

    # Verify installation
    if command -v xx >/dev/null 2>&1; then
        xx version
    else
        echo "Installation failed. xx not found. Is ${INSTALL_DIRECTORY} in your PATH?" >&2
        exit 1
    fi
}

install_xx_cli
