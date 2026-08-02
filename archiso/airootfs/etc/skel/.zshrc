# Omni-OS Default ZSH Configuration

# Path
export PATH=$HOME/.local/bin:/usr/local/bin:$PATH

# History
HISTFILE=~/.zsh_history
HISTSIZE=10000
SAVEHIST=10000
setopt appendhistory

# Basic aliases
alias ll='ls -alF'
alias la='ls -A'
alias l='ls -CF'
alias ls='ls --color=auto'
alias grep='ripgrep'
alias cat='bat --style=plain'

# Omni-OS aliases
alias update='sudo pacman -Syu'
alias install='sudo pacman -S'
alias remove='sudo pacman -Rns'
alias search='pacman -Ss'

# Editor
export EDITOR=nvim
export VISUAL=nvim

# Initialize Starship prompt
eval "$(starship init zsh)"

# Display fastfetch on login
if [[ -t 1 ]]; then
    fastfetch
fi
