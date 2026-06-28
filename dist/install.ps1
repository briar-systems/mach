# installs the mach release binary.
#
# MACH_VERSION      version to install (e.g. 1.2.3); defaults to the latest release
# MACH_INSTALL_DIR  install directory; set to skip the prompt; defaults to $env:LOCALAPPDATA\mach\bin
# MACH_BASE_URL     release base url override (for testing)

$ErrorActionPreference = 'Stop'

$base = if ($env:MACH_BASE_URL) { $env:MACH_BASE_URL } else { 'https://github.com/briar-systems/mach/releases' }
$target = 'x86_64-windows'

if (-not [System.Environment]::Is64BitOperatingSystem) {
    throw 'install.ps1: unsupported host; mach requires 64-bit windows'
}

# resolve the version to install: pinned via MACH_VERSION, else the latest tag
if ($env:MACH_VERSION) {
    $tag = if ($env:MACH_VERSION.StartsWith('v')) { $env:MACH_VERSION } else { "v$($env:MACH_VERSION)" }
} else {
    $handler = New-Object System.Net.Http.HttpClientHandler
    $handler.AllowAutoRedirect = $false
    $client = New-Object System.Net.Http.HttpClient($handler)
    try {
        $req = New-Object System.Net.Http.HttpRequestMessage([System.Net.Http.HttpMethod]::Head, "$base/latest")
        $resp = $client.SendAsync($req).GetAwaiter().GetResult()
        if (-not $resp.Headers.Location) { throw 'install.ps1: could not resolve the latest release tag' }
        $tag = ($resp.Headers.Location.ToString() -split '/tag/')[-1]
    } finally {
        $client.Dispose()
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
$mag   = if ([Console]::IsOutputRedirected) { '' } else { "${e}[38;2;255;0;255m" }
$reset = if ([Console]::IsOutputRedirected) { '' } else { "${e}[0m" }
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
    Invoke-WebRequest -Uri "$base/download/$tag/$archive" -OutFile (Join-Path $tmp $archive)

    Write-Host "extracting $archive..."
    Expand-Archive -Path (Join-Path $tmp $archive) -DestinationPath $tmp -Force
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
    Copy-Item (Join-Path $tmp 'mach.exe') (Join-Path $dir 'mach.exe') -Force
    Write-Host "`ninstalled mach $version to $(Join-Path $dir 'mach.exe')"
} finally {
    Remove-Item -Recurse -Force $tmp
}
