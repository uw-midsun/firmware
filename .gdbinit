# Disable signal interrupts from x86 interrupts
# SIGRTMIN + INTERRUPT_PRIORITY_LOW
handle SIG35 nostop noprint
# SIGRTMIN + INTERRUPT_PRIORITY_NORMAL
handle SIG36 nostop noprint
# SIGRTMIN + INTERRUPT_PRIORITY_HIGN
handle SIG37 nostop noprint
