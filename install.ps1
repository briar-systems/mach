# installs the mach release binary.
# usage: irm https://github.com/octalide/mach/releases/latest/download/install.ps1 | iex
#
# MACH_VERSION      version to install (e.g. 1.4.2); defaults to the latest release
# MACH_INSTALL_DIR  install directory; defaults to $env:LOCALAPPDATA\mach\bin
# MACH_BASE_URL     release base url override (for testing)

$ErrorActionPreference = 'Stop'

$base = if ($env:MACH_BASE_URL) { $env:MACH_BASE_URL } else { 'https://github.com/octalide/mach/releases' }
$dir = if ($env:MACH_INSTALL_DIR) { $env:MACH_INSTALL_DIR } else { Join-Path $env:LOCALAPPDATA 'mach\bin' }
$target = 'x86_64-windows'

if (-not [System.Environment]::Is64BitOperatingSystem) {
    throw 'install.ps1: unsupported host; mach requires 64-bit windows'
}

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

$archive = "mach-$version-$target.zip"
$tmp = Join-Path ([System.IO.Path]::GetTempPath()) "mach-install-$([System.IO.Path]::GetRandomFileName())"
New-Item -ItemType Directory -Path $tmp | Out-Null
try {
    Write-Host "downloading $archive ($tag)"
    Invoke-WebRequest -Uri "$base/download/$tag/$archive" -OutFile (Join-Path $tmp $archive)
    Invoke-WebRequest -Uri "$base/download/$tag/SHA256SUMS" -OutFile (Join-Path $tmp 'SHA256SUMS')

    $line = Select-String -Path (Join-Path $tmp 'SHA256SUMS') -Pattern ([regex]::Escape($archive) + '$') | Select-Object -First 1
    if (-not $line) { throw "install.ps1: no checksum for $archive in SHA256SUMS" }
    $expected = ($line.Line -split '\s+')[0].ToLower()
    $actual = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $tmp $archive)).Hash.ToLower()
    if ($actual -ne $expected) {
        throw "install.ps1: checksum verification FAILED for $archive (expected $expected, got $actual); aborting"
    }

    Expand-Archive -Path (Join-Path $tmp $archive) -DestinationPath $tmp -Force
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
    Copy-Item (Join-Path $tmp 'mach.exe') (Join-Path $dir 'mach.exe') -Force
    Write-Host "installed mach $version to $(Join-Path $dir 'mach.exe')"

    if (-not (($env:Path -split ';') -contains $dir)) {
        Write-Host "note: $dir is not on PATH; add it, e.g.:"
        Write-Host "  [Environment]::SetEnvironmentVariable('Path', `"$dir;`" + [Environment]::GetEnvironmentVariable('Path', 'User'), 'User')"
    }
} finally {
    Remove-Item -Recurse -Force $tmp
}
