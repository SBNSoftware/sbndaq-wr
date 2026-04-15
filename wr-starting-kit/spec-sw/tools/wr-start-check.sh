#!/bin/bash
: << SPECIFICATION
wr_start_check.sh [--sender|--receiver]

Check and error out if any are not true:
The script must be run as root.
Check that the system has hyperthreading disabled.
check that the system has one CPU isolated via /proc/cmdline:
BOOT_IMAGE+=... isolcpus=15 nohz_full=15 rcu_nocbs=15
The isolated cpu should be retained in the CPU shell variable.

The script should check if the wr_nic module and it's support spec and fmc
modules are installed.
If they are not installed, to installed them:
  check, in order for spec and fmc and if either are installed, rmmod.
  then: systemctl start spec
    and check that the wr0 interface is UP.

Then check if on short hostaneme icarus-clk04 or icarus-clk06:
icarus-clk04 is the default host for the "sender" node
and icarus-clk06 is the default for the "receiver" node
If on neither of these, and neiter, but not both, the --sender or
--receiver
option are given, the current node can be used for a sender or receiver.

For both the sender an receiver nodes, the wr-nic interrupt should be
assigned to the isolated core, check if it is and make it happen if not.
This isolated core should have the C* states disabled as such:
    for state in /sys/devices/system/cpu/cpu$CPU/cpuidle/state*;do
      name=`cat $state/name`
      if expr $name : C;then
        grep 1 $state/disable && echo Current $name: disabled || {
          echo 1 >|$state/disable
          echo Disabled $name
        }
      fi
    done

Or something similar.

For the sender node, there should be 2 instances of the wr-dio-ruler
command running and pinned to the isolated CPU and running with a real-time
scheduling policy.
One of them should have arguments: wr0 IN1 R1+0.034697225
                    and the other: wr0 IN4 R4+0.000331005
 
For the receiving node, there should be one instance of the command
wr-dio-agent running and this should have the argument: wr0
It should be pinned to the isolated CPU with a real-time scheduling policy.

SPECIFICATION

set -e

# ============================================================================
# Configuration
# ============================================================================
RT_PRIORITY=80
IN1_OFFSET="R1+0.034697225"
IN4_OFFSET="R4+0.000331005"
WR_INTERFACE="wr0"

# ============================================================================
# Helper functions
# ============================================================================
die() {
    echo "ERROR: $*" >&2
    exit 1
}

info() {
    echo "INFO: $*"
}

fix_msg() {
    echo "FIXING: $*"
}

# ============================================================================
# Check: Must be root
# ============================================================================
[[ $EUID -eq 0 ]] || die "This script must be run as root"

# ============================================================================
# Check: Hyperthreading disabled
# ============================================================================
if grep -q "^1$" /sys/devices/system/cpu/smt/active 2>/dev/null; then
    die "Hyperthreading is enabled. Please disable it in BIOS or via: echo off > /sys/devices/system/cpu/smt/control"
fi
info "Hyperthreading is disabled"

# ============================================================================
# Check: Isolated CPU from /proc/cmdline
# ============================================================================
CMDLINE=$(cat /proc/cmdline)
CPU=""
if [[ $CMDLINE =~ isolcpus=([0-9]+) ]]; then
    CPU=${BASH_REMATCH[1]}
else
    die "No isolcpus= found in /proc/cmdline. Boot with isolcpus=N nohz_full=N rcu_nocbs=N"
fi

# Verify nohz_full and rcu_nocbs match
if ! [[ $CMDLINE =~ nohz_full=$CPU ]]; then
    die "nohz_full=$CPU not found in /proc/cmdline"
fi
if ! [[ $CMDLINE =~ rcu_nocbs=$CPU ]]; then
    die "rcu_nocbs=$CPU not found in /proc/cmdline"
fi
info "Isolated CPU: $CPU (isolcpus, nohz_full, rcu_nocbs all set)"

# ============================================================================
# Check/Install: wr_nic module and dependencies (spec, fmc)
# ============================================================================
install_modules() {
    # Remove in reverse dependency order if present
    for mod in wr_nic spec fmc; do
        if lsmod | grep -qw "^$mod"; then
            fix_msg "Removing module $mod"
            rmmod $mod || true
        fi
    done
    
    fix_msg "Starting spec service"
    systemctl start spec
    
    # Wait for interface to come up
    local tries=10
    while [[ $tries -gt 0 ]]; do
        if ip link show $WR_INTERFACE 2>/dev/null | grep -q "UP"; then
            info "$WR_INTERFACE interface is UP"
            return 0
        fi
        sleep 1
        ((tries--))
    done
    die "$WR_INTERFACE interface did not come UP after starting spec service"
}

if ! lsmod | grep -qw "^wr_nic"; then
    fix_msg "wr_nic module not loaded, installing modules"
    install_modules
else
    info "wr_nic module is loaded"
    # Verify interface is up
    if ! ip link show $WR_INTERFACE 2>/dev/null | grep -q "UP"; then
        die "$WR_INTERFACE interface exists but is not UP"
    fi
    info "$WR_INTERFACE interface is UP"
fi

# ============================================================================
# Determine node role: sender or receiver
# ============================================================================
MODE=""
SHORT_HOST=$(hostname -s)

while [[ $# -gt 0 ]]; do
    case $1 in
        --sender)
            [[ -z $MODE ]] || die "Cannot specify both --sender and --receiver"
            MODE="sender"
            shift
            ;;
        --receiver)
            [[ -z $MODE ]] || die "Cannot specify both --sender and --receiver"
            MODE="receiver"
            shift
            ;;
        *)
            die "Unknown option: $1"
            ;;
    esac
done

if [[ -z $MODE ]]; then
    case $SHORT_HOST in
        icarus-clk04)
            MODE="sender"
            info "Detected sender node (icarus-clk04)"
            ;;
        icarus-clk06)
            MODE="receiver"
            info "Detected receiver node (icarus-clk06)"
            ;;
        *)
            die "Unknown host '$SHORT_HOST'. Use --sender or --receiver to specify role."
            ;;
    esac
else
    info "Mode set via command line: $MODE"
fi

# ============================================================================
# Assign wr-nic interrupt to isolated CPU
# ============================================================================
assign_irq_affinity() {
    local irq_dir found=0
    for irq_dir in /proc/irq/*/; do
        if [[ -f "${irq_dir}smp_affinity_list" ]]; then
            # Check if this IRQ belongs to wr-nic or spec
            if grep -qE "(wr|spec)" "${irq_dir}"*actions 2>/dev/null || \
               [[ -d "${irq_dir}" ]] && ls "${irq_dir}" 2>/dev/null | grep -qE "(wr|spec)"; then
                local irq_num=$(basename "$irq_dir")
                local current=$(cat "${irq_dir}smp_affinity_list" 2>/dev/null)
                if [[ "$current" != "$CPU" ]]; then
                    fix_msg "Setting IRQ $irq_num affinity to CPU $CPU (was: $current)"
                    echo $CPU > "${irq_dir}smp_affinity_list"
                else
                    info "IRQ $irq_num already assigned to CPU $CPU"
                fi
                found=1
            fi
        fi
    done
    
    # Alternative: check by driver name in /sys
    if [[ $found -eq 0 ]]; then
        for irq in $(grep -E "spec|wr" /proc/interrupts | awk '{print $1}' | tr -d ':'); do
            local current=$(cat /proc/irq/$irq/smp_affinity_list 2>/dev/null)
            if [[ "$current" != "$CPU" ]]; then
                fix_msg "Setting IRQ $irq affinity to CPU $CPU (was: $current)"
                echo $CPU > /proc/irq/$irq/smp_affinity_list
            else
                info "IRQ $irq already assigned to CPU $CPU"
            fi
            found=1
        done
    fi
    
    [[ $found -eq 1 ]] || die "Could not find wr-nic/spec IRQ to assign"
}

assign_irq_affinity

# ============================================================================
# Disable C-states on isolated CPU
# ============================================================================
disable_cstates() {
    for state in /sys/devices/system/cpu/cpu$CPU/cpuidle/state*; do
        [[ -d $state ]] || continue
        local name=$(cat "$state/name")
        if [[ $name =~ ^C ]]; then
            if grep -q "^1$" "$state/disable" 2>/dev/null; then
                info "C-state $name already disabled on CPU $CPU"
            else
                fix_msg "Disabling C-state $name on CPU $CPU"
                echo 1 >| "$state/disable"
            fi
        fi
    done
}

disable_cstates

# ============================================================================
# Start/verify wr-dio processes based on role
# ============================================================================
start_ruler() {
    local input=$1
    local offset=$2
    
    # Check if already running with correct args
    if pgrep -f "wr-dio-ruler $WR_INTERFACE $input $offset" >/dev/null; then
        info "wr-dio-ruler for $input already running"
        return 0
    fi
    
    # Kill any existing ruler for this input
    pkill -f "wr-dio-ruler.*$input" 2>/dev/null || true
    
    fix_msg "Starting wr-dio-ruler $WR_INTERFACE $input $offset on CPU $CPU with RT priority $RT_PRIORITY"
    taskset -c $CPU chrt -f $RT_PRIORITY /bin/wr-dio-ruler $WR_INTERFACE $input $offset &
    disown
    sleep 0.5
    
    if ! pgrep -f "wr-dio-ruler $WR_INTERFACE $input $offset" >/dev/null; then
        die "Failed to start wr-dio-ruler for $input"
    fi
}

start_agent() {
    # Check if already running
    if pgrep -f "wr-dio-agent $WR_INTERFACE" >/dev/null; then
        info "wr-dio-agent already running"
        return 0
    fi
    
    # Kill any existing agent
    pkill -f "wr-dio-agent" 2>/dev/null || true
    
    fix_msg "Starting wr-dio-agent $WR_INTERFACE on CPU $CPU with RT priority $RT_PRIORITY"
    taskset -c $CPU chrt -f $RT_PRIORITY /bin/wr-dio-agent $WR_INTERFACE &
    disown
    sleep 0.5
    
    if ! pgrep -f "wr-dio-agent $WR_INTERFACE" >/dev/null; then
        die "Failed to start wr-dio-agent"
    fi
}

case $MODE in
    sender)
        info "Configuring sender node"
        start_ruler "IN1" "$IN1_OFFSET"
        start_ruler "IN4" "$IN4_OFFSET"
        ;;
    receiver)
        info "Configuring receiver node"
        start_agent
        ;;
esac

# ============================================================================
# Summary
# ============================================================================
echo ""
echo "============================================"
echo "wr-start-check.sh completed successfully"
echo "  Mode: $MODE"
echo "  Isolated CPU: $CPU"
echo "  Interface: $WR_INTERFACE"
echo "============================================"

