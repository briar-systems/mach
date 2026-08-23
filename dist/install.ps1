# installs the mach release binary.
#
# MACH_VERSION      version to install (e.g. 1.2.3); defaults to the latest release
# MACH_INSTALL_DIR  install directory; set to skip the prompt; defaults to $env:LOCALAPPDATA\mach\bin
# MACH_BASE_URL     release base url override (for testing)

$ErrorActionPreference = 'Stop'

# windows powershell 5.1 can start with a SecurityProtocol set that excludes
# TLS 1.2, and github refuses anything older; or it in rather than overwrite
if ($PSVersionTable.PSVersion.Major -lt 6) {
    [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
}

$base = if ($env:MACH_BASE_URL) { $env:MACH_BASE_URL } else { 'https://github.com/briar-systems/mach/releases' }
$target = 'x86_64-windows'

if (-not [System.Environment]::Is64BitOperatingSystem) {
    throw 'install.ps1: unsupported host; mach requires 64-bit windows'
}

# resolve the version to install: pinned via MACH_VERSION, else the latest tag
if ($env:MACH_VERSION) {
    $tag = if ($env:MACH_VERSION.StartsWith('v')) { $env:MACH_VERSION } else { "v$($env:MACH_VERSION)" }
} else {
    # HttpWebRequest lives in System.dll, loaded in every powershell; the
    # HttpClient it replaces needed System.Net.Http, absent by default on
    # windows powershell 5.1 (#3086). a redirect status is not an error to
    # GetResponse, so the Location header comes back without an exception.
    $req = [System.Net.WebRequest]::CreateHttp("$base/latest")
    $req.Method = 'HEAD'
    $req.AllowAutoRedirect = $false
    $resp = $req.GetResponse()
    try {
        $loc = $resp.Headers['Location']
        if (-not $loc) { throw 'install.ps1: could not resolve the latest release tag' }
        $tag = ($loc -split '/tag/')[-1]
    } finally {
        $resp.Close()
    }
}
$version = $tag.TrimStart('v')

$art = @'
                        _
  _ __ ___    __ _  ___| |__
 | '_ ` _ \  / _` |/ __| '_ \
 | | | | | || (_| | (__| | | |
 |_| |_| |_| \__,_|\___|_| |_|
'@
# the banner is the only colored output: mach magenta (0xff00ff) on a real console
$e = [char]27
# 5.1's console leaves VT processing off, so the escapes would print literally
$vt    = -not [Console]::IsOutputRedirected -and $Host.UI.SupportsVirtualTerminal
$mag   = if ($vt) { "${e}[38;2;255;0;255m" } else { '' }
$reset = if ($vt) { "${e}[0m" } else { '' }
Write-Host ''
Write-Host "$mag$art$reset"
Write-Host "  mach $version ($target)`n"

# install directory: explicit env override wins; otherwise prompt at a console,
# falling back to the default when input is redirected (e.g. CI)
$default = Join-Path $env:LOCALAPPDATA 'mach\bin'
if ($env:MACH_INSTALL_DIR) {
    $dir = $env:MACH_INSTALL_DIR
} elseif (-not [Console]::IsInputRedirected) {
    $reply = Read-Host "install directory [$default]"
    $dir = if ([string]::IsNullOrWhiteSpace($reply)) { $default } else { $reply }
} else {
    $dir = $default
}

$archive = "mach-$version-$target.zip"
$tmp = Join-Path ([System.IO.Path]::GetTempPath()) "mach-install-$([System.IO.Path]::GetRandomFileName())"
New-Item -ItemType Directory -Path $tmp | Out-Null
try {
    Write-Host "`ndownloading $archive..."
    # the progress bar makes 5.1's download an order of magnitude slower; the
    # scriptblock keeps the preference change out of the caller's session
    & {
        $ProgressPreference = 'SilentlyContinue'
        Invoke-WebRequest -UseBasicParsing -Uri "$base/download/$tag/$archive" -OutFile (Join-Path $tmp $archive)
    }

    Write-Host "extracting $archive..."
    Expand-Archive -Path (Join-Path $tmp $archive) -DestinationPath $tmp -Force
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
    Copy-Item (Join-Path $tmp 'mach.exe') (Join-Path $dir 'mach.exe') -Force
    Write-Host "`ninstalled mach $version to $(Join-Path $dir 'mach.exe')"
} finally {
    Remove-Item -Recurse -Force $tmp
}
