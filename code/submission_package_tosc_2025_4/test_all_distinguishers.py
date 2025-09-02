#!/usr/bin/env python3
"""
Test script for all ChiLow integral distinguishers claimed in the paper.

This script runs the integral cryptanalysis tool for all 12 distinguishers
and verifies that they work as expected.

Author: Hosein Hadipour <hsn.hadipour@gmail.com>
Date: September 2025
"""

import subprocess
import sys
import os

# All distinguishers from the paper
DISTINGUISHERS = [
    ([21, 23, 25], [2, 3, 14, 25, 26]),
    ([27, 29, 31], [5, 14, 22, 25, 28]),
    ([3, 5, 7], [0, 4, 24]),
    ([6, 8, 10], [5, 24, 27]),
    ([7, 9, 11], [0, 9, 12]),
    ([9, 11, 13], [7, 11, 31]),
    ([12, 14, 17], [6, 21, 24]),
    ([17, 19, 21], [14, 19, 23]),
    ([18, 20, 22], [6, 18, 27]),
    ([19, 21, 25], [2, 14, 26]),
    ([22, 24, 26], [21, 25, 30]),
    ([26, 28, 30], [1, 10, 22])
]

def test_distinguisher(active_bits, balanced_bits, rounds=3, repetitions=20):
    """Test a single integral distinguisher."""
    
    # Format bit lists as comma-separated strings
    active_str = ",".join(map(str, active_bits))
    balanced_str = ",".join(map(str, balanced_bits))
    
    print(f"Testing distinguisher: Active=[{active_str}], Balanced=[{balanced_str}]")
    
    # Run the integral tool
    cmd = [
        "./build/integral",
        str(rounds),
        active_str,
        balanced_str,
        str(repetitions)
    ]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        
        if result.returncode != 0:
            print(f"[ERROR] Command failed with return code {result.returncode}")
            print(f"stderr: {result.stderr}")
            return False
        
        # Check if the distinguisher was confirmed
        if "INTEGRAL DISTINGUISHER CONFIRMED" in result.stdout:
            # Extract success rate
            lines = result.stdout.split('\n')
            for line in lines:
                if "Successful repetitions:" in line:
                    if "100.0%" in line:
                        print("[SUCCESS] All balanced bits confirmed for 3 rounds")
                        return True
                    else:
                        print(f"[PARTIAL] {line.split('Successful repetitions:')[1].strip()}")
                        return False
        else:
            print("[FAILED] Distinguisher not confirmed")
            return False
    
    except subprocess.TimeoutExpired:
        print("[ERROR] Test timed out")
        return False
    except Exception as e:
        print(f"[ERROR] {e}")
        return False

def main():
    """Run all distinguisher tests."""
    
    print("ChiLow Integral Distinguisher Verification")
    print("==========================================")
    print("Testing all 12 distinguishers from the paper...\n")
    
    # Check if integral tool exists
    if not os.path.exists("./build/integral"):
        print("[ERROR] Integral tool not found. Please run 'make integral' first.")
        sys.exit(1)
    
    passed = 0
    total = len(DISTINGUISHERS)
    
    for i, (active, balanced) in enumerate(DISTINGUISHERS, 1):
        print(f"Test {i}/{total}:")
        if test_distinguisher(active, balanced):
            passed += 1
        print()
    
    print("=" * 50)
    print(f"Results: {passed}/{total} distinguishers confirmed")
    
    if passed == total:
        print("[SUCCESS] All distinguishers from the paper verified!")
        sys.exit(0)
    else:
        print(f"[WARNING] {total - passed} distinguisher(s) failed")
        sys.exit(1)

if __name__ == "__main__":
    main()
