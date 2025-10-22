import streamlit as st
import random
import numpy as np
import matplotlib.pyplot as plt
import ctypes
import sys
import os

st.title("Dynamic Hash Table Analyzer with C Backend")

# Load the C library

if sys.platform.startswith("win"):
    libname = "hashtable.dll"
else:
    libname = "./libhashtable.so"

lib = ctypes.CDLL(libname)

# Define C struct Stats

class Stats(ctypes.Structure):
    _fields_ = [("total_inserts", ctypes.c_int),
                ("total_collisions", ctypes.c_int),
                ("total_probes", ctypes.c_int)]

lib.run_hash_test.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_int]
lib.run_hash_test.restype = Stats

# Constants matching C enum

LINEAR_PROBING = 1
SEPARATE_CHAINING = 2

collision_methods = {
    "Linear Probing": LINEAR_PROBING,
    "Separate Chaining": SEPARATE_CHAINING,
}

collision_method = st.selectbox("Select Collision Resolution Method", list(collision_methods.keys()))
load_factor = st.slider("Load Factor", 0.1, 0.9, 0.5)
data_distribution = st.selectbox("Select Data Distribution", ["Uniform", "Clustered", "Skewed"])

TABLE_SIZE = 1009
num_elements = int(TABLE_SIZE * load_factor)

# Data generation

def generate_data(n, distribution):
    if distribution == "Uniform":
        return random.sample(range(1, TABLE_SIZE*10), n)
    elif distribution == "Clustered":
        center = TABLE_SIZE * 5
        return [random.randint(center-20, center+20) for _ in range(n)]
    else:  # Skewed, exponential
        return [int(random.expovariate(1/(TABLE_SIZE/5))) for _ in range(n)]

data = generate_data(num_elements, data_distribution)

# Convert data to ctypes array

IntArray = ctypes.c_int * len(data)
c_data = IntArray(*data)

method_c = collision_methods[collision_method]

# Run the test

stats = lib.run_hash_test(c_data, len(data), method_c)

st.write(f"Total keys inserted: {stats.total_inserts}")
st.write(f"Total collisions: {stats.total_collisions}")
if collision_method == "Linear Probing":
    st.write(f"Total probes: {stats.total_probes}")
    avg_probes = stats.total_probes / stats.total_inserts if stats.total_inserts > 0 else 0
    st.write(f"Average probes per insertion: {avg_probes:.2f}")

# Plot collisions over varying load factors

def simulate_load_curve(method_c, distribution, load_factors):
    collisions_list = []
    probes_list = []
    for lf in load_factors:
        n = int(TABLE_SIZE * lf)
        d = generate_data(n, distribution)
        arr = IntArray(*d)
        s = lib.run_hash_test(arr, len(d), method_c)
        collisions_list.append(s.total_collisions)
        probes_list.append(s.total_probes)
    return collisions_list, probes_list

load_factors = [x/10 for x in range(1,10)]
collisions, probes = simulate_load_curve(method_c, data_distribution, load_factors)

fig, ax = plt.subplots()
ax.plot(load_factors, collisions, label="Collisions", marker='o')
if collision_method == "Linear Probing":
    ax.plot(load_factors, [p/(TABLE_SIZE*lf) if lf>0 else 0 for p,lf in zip(probes, load_factors)], label="Avg probes", marker='x')
ax.set_xlabel("Load Factor")
ax.set_ylabel("Count")
ax.set_title(f"Collisions and Avg Probes vs Load Factor ({collision_method})")
ax.legend()
ax.grid()
st.pyplot(fig)
