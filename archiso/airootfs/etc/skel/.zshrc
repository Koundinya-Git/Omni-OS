export PATH=$HOME/.local/bin:/usr/local/bin:$PATH

HISTFILE=~/.zsh_history
HISTSIZE=10000
SAVEHIST=10000
setopt appendhistory

alias ll='ls -alF'
alias la='ls -A'
alias l='ls -CF'
alias ls='ls --color=auto'
alias grep='ripgrep'
alias cat='bat --style=plain'

alias update='sudo pacman -Syu'
alias install='sudo pacman -S'
alias remove='sudo pacman -Rns'
alias search='pacman -Ss'

export EDITOR=nvim
export VISUAL=nvim

eval "$(starship init zsh)"

if [[ -t 1 ]]; then
    fastfetch
fi
