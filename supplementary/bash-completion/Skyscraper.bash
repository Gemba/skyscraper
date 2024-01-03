# Programmable completion for Skyscraper: https://gemba.github.io/skyscraper/
#
# This file is part of skyscraper. Copyright (c) 2024 Gemba @ GitHub
#
# skyscraper is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# skyscraper is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with skyscraper; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
#

_skyscraper_parse_subhelp() {
	local executable="$1"
	local prev="$2"
	# assume RetroPie install
	local skyscraper_bin="/opt/retropie/supplementary/skyscraper/$executable"
	if [[ ! -e "$skyscraper_bin" ]]; then
		# other Linux install
		skyscraper_bin="/usr/local/bin/$executable"
		if [[ ! -e "$skyscraper_bin" ]]; then
			return
		fi
	fi
	# detect with shell coloring but remove shell coloring before output
	"$skyscraper_bin" "$prev" help | awk '/  [a-z](.+)\x1b\[[0-9;]*m/ {print $2}' | sed -e 's/\x1b\[[0-9;]*m//g'
}

_skyscraper_compgen_fn() {
	local cur="$1"
	# files, excluding directories
	grep -v -F -f <(compgen -d -P ^ -S '$' -- "$cur") <(compgen -f -P ^ -S '$' -- "$cur") | sed -e 's/^\^//' -e 's/\$$/ /'
	# append output with directories
	compgen -d -S / -- "$cur"
}

_skyscraper() {
	local cur prev
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD - 1]}"

	case $prev in
	'-p')
		# platforms
		local IFS=$'\n'
		# try from config.ini, [main] section only
		local input_folder
		input_folder=$(
			python3 -c "import configparser; c = configparser.ConfigParser(); c.read('$HOME/.skyscraper/config.ini'); print(c['main']['inputFolder'].replace('\"',''))" 2>/dev/null
		)
		COMPREPLY=()
		if { [[ -z "$input_folder" ]] || [[ ! -d "$input_folder" ]]; } && [[ -d "$HOME/RetroPie/roms" ]]; then
			# set failsafe on RetroPie install
			input_folder="$HOME/RetroPie/roms"
		fi

		# no inputfolder, no completion
		[[ -z "$input_folder" ]] && return 0

		input_folder="${input_folder%/}/"
		mapfile -t _dirs < <(cd "$input_folder" && compgen -d -- "$cur")
		for d in "${_dirs[@]}"; do
			_f=$(
				shopt -s nullglob dotglob
				echo "$input_folder/$d"/*
			)
			# exclude: empty dirs, symlinks and dotdirs
			if ((${#_f})) && [[ ! -L "$input_folder/$d" ]] && [[ ! "$d" =~ ^\. ]]; then
				COMPREPLY+=("$d")
			fi
		done
		return 0
		;;
	'-s')
		# scrape modules
		mapfile -t COMPREPLY < <(compgen -W "arcadedb igdb mobygames openretro screenscraper thegamesdb worldofspectrum esgamelist import" -- "$cur")
		return 0
		;;
	'-f')
		# frontends
		mapfile -t COMPREPLY < <(compgen -W "emulationstation pegasus retrobat attractmode" -- "$cur")
		return 0
		;;
	'-t')
		mapfile -t COMPREPLY < <(compgen -W "$(seq "$(nproc)")" -- "$cur")
		return 0
		;;
	'-d' | '-g' | '-i' | '-o')
		compopt -o nospace
		mapfile -t COMPREPLY < <(compgen -d -S / -- "$cur")
		return 0
		;;
	'-a' | '-c' | '--excludefrom' | '--includefrom' | '--endat' | '--startat')
		compopt -o nospace
		mapfile -t COMPREPLY < <(_skyscraper_compgen_fn "$cur" -- "$cur")
		return 0
		;;
	'-h' | '--help' | '--help-all' | '-v' | '--version')
		return 0
		;;
	'--verbosity')
		mapfile -t COMPREPLY < <(compgen -W "0 1 2 3" -- "$cur")
		return 0
		;;
	'-e' | '-l' | '-m' | '-u' | '--addext' | '--query' | '--maxfails' | '--lang' | '--region' | '--includepattern' | '--exludepattern')
		# expect specific values
		return 0
		;;
	'--cache' | '--flags')
		# complete with cache/flags options
		local sub_opts
		sub_opts=$(_skyscraper_parse_subhelp "$1" "$prev")
		mapfile -t COMPREPLY < <(compgen -W "${sub_opts}" -- "$cur")
		return 0
		;;
	esac

	if [[ $cur == -* ]]; then
		# complete options
		local opts
		opts="$(_parse_help "$1")"
		mapfile -t COMPREPLY < <(compgen -W "${opts}" -- "$cur")
		return 0
	fi
	# complete filename (aka rom name)
	mapfile -t COMPREPLY < <(_skyscraper_compgen_fn "$cur" -- "$cur")
} &&
	complete -F _skyscraper -o default Skyscraper
