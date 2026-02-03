#!/bin/bash

readonly RELEASES_URL="https://api.github.com/repos/gallardo994/xx-cli/releases"
readonly INSTALL_DIRECTORY="/usr/local/bin"
readonly BINARY_NAME="xx"
readonly DOWNLOAD_URL_TEMPLATE="https://github.com/Gallardo994/xx-cli/releases/download/%s/xx-%s-%s-release"

readonly SUPPORTED_MACOS_ARCHS=("arm64")
readonly SUPPORTED_LINUX_ARCHS=("x86_64" "arm64")

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
        darwin) PLATFORM="macos" ;;
        linux) PLATFORM="linux" ;;
        *)
            echo "Unsupported platform: ${PLATFORM}. Cancelling." >&2
            exit 1
            ;;
    esac

    if [ "${PLATFORM}" = "macos" ]; then
        if [[ ! " ${SUPPORTED_MACOS_ARCHS[*]} " =~ " ${ARCH} " ]]; then
            echo "Unsupported architecture for macOS: ${ARCH}. Cancelling." >&2
            exit 1
        fi
    elif [ "${PLATFORM}" = "linux" ]; then
        if [[ ! " ${SUPPORTED_LINUX_ARCHS[*]} " =~ " ${ARCH} " ]]; then
            echo "Unsupported architecture for Linux: ${ARCH}. Cancelling." >&2
            exit 1
        fi
    fi

    DOWNLOAD_URL=$(printf "${DOWNLOAD_URL_TEMPLATE}" "${LATEST_RELEASE}" "${PLATFORM}" "${ARCH}")
    DOWNLOAD_TEMP_FILE="$(mktemp)"

    printf "Downloading ${PLATFORM}-${ARCH} from %s\n" "${DOWNLOAD_URL}"
    curl -L -o "${DOWNLOAD_TEMP_FILE}" "${DOWNLOAD_URL}"
    chmod +x "${DOWNLOAD_TEMP_FILE}"
    printf "File downloaded to %s\n" "${DOWNLOAD_TEMP_FILE}"

    printf "Installing xx-cli to %s\n" "${INSTALL_DIRECTORY}/${BINARY_NAME}"
    sudo mv "${DOWNLOAD_TEMP_FILE}" "${INSTALL_DIRECTORY}/${BINARY_NAME}"

    if command -v xx >/dev/null 2>&1; then
        xx version
    else
        echo "Installation failed. xx not found. Is ${INSTALL_DIRECTORY} in your PATH?" >&2
        exit 1
    fi
}

install_xx_cli
