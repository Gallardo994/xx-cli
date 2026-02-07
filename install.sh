#!/bin/bash

readonly RELEASES_URL="https://api.github.com/repos/gallardo994/xx-cli/releases"
readonly INSTALL_DIRECTORY="/usr/local/bin"
readonly BINARY_NAME="xx"
readonly DOWNLOAD_URL_TEMPLATE="https://github.com/Gallardo994/xx-cli/releases/download/%s/xx-%s-%s-release"

readonly SUPPORTED_MACOS_ARCHS=("arm64")
readonly SUPPORTED_LINUX_ARCHS=("x86_64" "arm64")

has_command() {
    command -v "$1" >/dev/null 2>&1
}

download_file() {
    local url="$1"
    local output="$2"

    if has_command curl; then
        printf "Using curl"
        curl -sSL "${url}" -o "${output}"
        return $?
    elif has_command wget; then
        printf "Using wget"
        wget -q -O "${output}" "${url}"
        return $?
    else
        echo "Error: Neither curl nor wget is installed. Please install one of them and try again." >&2
        return 1
    fi
}

get_latest_release() {
    local curl_or_wget_args

    if has_command curl; then
        curl -sSL "${RELEASES_URL}/latest" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/'
    elif has_command wget; then
        wget -qO- "${RELEASES_URL}/latest" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/'
    else
        echo ""
    fi
}

install_xx_cli() {
    local LATEST_RELEASE
    LATEST_RELEASE=$(get_latest_release)

    if [ -z "${LATEST_RELEASE}" ]; then
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

    local DOWNLOAD_URL
    DOWNLOAD_URL=$(printf "${DOWNLOAD_URL_TEMPLATE}" "${LATEST_RELEASE}" "${PLATFORM}" "${ARCH}")
    local DOWNLOAD_TEMP_FILE
    DOWNLOAD_TEMP_FILE="$(mktemp)"

    printf "Downloading ${PLATFORM}-${ARCH} from %s\n" "${DOWNLOAD_URL}"

    if ! download_file "${DOWNLOAD_URL}" "${DOWNLOAD_TEMP_FILE}"; then
        rm -f "${DOWNLOAD_TEMP_FILE}"
        exit 1
    fi

    chmod +x "${DOWNLOAD_TEMP_FILE}"
    printf "File downloaded to %s\n" "${DOWNLOAD_TEMP_FILE}"

    printf "Installing xx-cli to %s\n" "${INSTALL_DIRECTORY}/${BINARY_NAME}"
    su root -c "mv \"${DOWNLOAD_TEMP_FILE}\" \"${INSTALL_DIRECTORY}/${BINARY_NAME}\""

    if command -v xx >/dev/null 2>&1; then
        xx version
    else
        exit 1
    fi
}

install_xx_cli
