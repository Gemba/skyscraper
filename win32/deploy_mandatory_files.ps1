param([string]$src_dir,[string]$skyscraper_home)

# These may be customized by user, thus do not overwrite with the repo files. 
# The (newer) repo file will then get '.dist' extension appended in target dir.
$user_cfg_files = @(
  'artwork.xml'
  'aliasMap.csv'
  'peas.json'
  'platforms_idmap.csv'
)

$folders = @(
  '.',
  'cache'
  'import'
  'import/covers'
  'import/maquees'
  'import/screenshots'
  'import/textual'
  'import/textures'
  'import/videos'
  'import/wheels'
  'resources'
)

$dmap = @{
  'config' = '.'
  'examples' = '.'
  'cacheexamples' = 'cache'
  'impexamples' = 'import'
  'resexamples' = 'resources'
}
# ignore: unix.supplementary.files and unix.target.files

# key: value from dmap, value: unix.($dmap.key).files= from skyscraper.pro
$fmap = @{}

# Prepare map $fmap with files to copy for each folder
$regex = 'unix:(?<key>[^.]+)\.files=(?<deployfiles>.+?)unix'

(Get-Content $src_dir/skyscraper.pro) -join ' ' -replace '\\' |
Select-String $regex -AllMatches | ForEach-Object {
  $g = $_.Matches.Groups |
  Where-Object { $_.Name -eq 'key' -or $_.Name -eq 'deployfiles' } |
  Select-Object -Property Value
  for ($i = 0; $i -lt $g.length; $i += 2) {
    $v = $g[$i].Value.Trim()
    if (-not [string]::IsNullOrEmpty($v) -and $dmap.ContainsKey($v)) {
      $deployfiles = ($g[$i + 1].Value -replace ('\s+',' ')).Trim().Split(' ')
      if ($fmap.ContainsKey($dmap.$v)) {
        $deployfiles += $fmap.($dmap.$v)
        $fmap.($dmap.$v) = $deployfiles
      } else {
        $fmap.Add($dmap.$v,$deployfiles)
      }
    }
  }
}

# Copy files
$folders | ForEach-Object {
  $subfolder = $_
  mkdir -Force "$skyscraper_home/$subfolder" | Out-Null
  if ($fmap.ContainsKey($subfolder)) {
    "[*] Install folder '$subfolder':"
    $fmap.$subfolder | Sort-Object –CaseSensitive | ForEach-Object {
      $cf = $_.Split('/')[-1]
      if (($cf -in $user_cfg_files) -and `
           (Test-Path $skyscraper_home/$subfolder/$cf)) {
        $cf_dist = "{0}.dist" -f $cf
        $cf_print = "$skyscraper_home/$subfolder/$cf"
        $cf_print = $cf_print.Replace("/","\").Replace("\.\","\")
        "    {0} exists. Copying new file to $cf_dist" -f $cf_print
        Copy-Item -Force -Path "$src_dir/$_" `
           "$skyscraper_home/$subfolder/$cf_dist"
      } else {
        $cf_print = (("$skyscraper_home/$subfolder".Replace("/","\")) `
             -replace ('\.$','')) -replace ('([a-z])$','$1\')
        "    Copy $_ to $cf_print"
        Copy-Item -Path "$src_dir/$_" "$skyscraper_home/$subfolder"
      }
    }
  }
}
