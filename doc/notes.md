# Notes on Improving `remove_dominated_solutions`

The `remove_dominated_solutions` function is a critical component, and its current `O(N^2)` complexity can become a performance bottleneck for large sets of solutions. To address this, a more efficient non-dominated sorting algorithm is proposed.

## Current Approach (Conceptual `O(N^2)`)

The existing logic for `remove_dominated_solutions` (specifically the overload that takes only the `solutions` vector) can be conceptually described as:

```cpp
// For each solution 's1' in the input list:
//   Assume 's1' is non-dominated.
//   For each other solution 's2' in the input list:
//     If 's2' dominates 's1':
//       Mark 's1' as dominated.
//       Break (no need to check further for 's1').
//   If 's1' is still non-dominated, add it to the result.
```

This nested loop structure directly leads to the `O(N^2)` time complexity, where N is the number of solutions.

## Proposed Improvement: Fast Non-dominated Sort

A robust and widely used algorithm for identifying non-dominated solutions is the **Fast Non-dominated Sort**. While its worst-case time complexity remains `O(N^2)` (or `O(M*N^2)` where M is the number of objectives, if explicitly considered), it provides a more structured approach and often performs better in practice. It also serves as a foundational component for more advanced multi-objective optimization algorithms.

### Algorithm Steps

1. **Initialization**:
    - For each solution `p` in the input `solutions` vector, we need to calculate two values:
        - `np` (domination count): The number of solutions that dominate `p`.
        - `Sp` (dominated set): A list of solutions that `p` dominates.

2. **First Front Identification**:
    - Iterate through all solutions. Any solution `p` for which `np` is 0 (meaning no other solution dominates it) belongs to the first Pareto front (the non-dominated set). Collect these solutions.

3. **Subsequent Fronts (Optional for this specific function)**:
    - (For `remove_dominated_solutions`, we only care about the first front. However, for completeness of the Fast Non-dominated Sort algorithm, subsequent fronts are identified by iteratively processing solutions dominated by the current front and decrementing their `np` counts. When an `np` count drops to 0, that solution belongs to the next front.)

### Implementation Details

To implement this, the `remove_dominated_solutions` function would be modified as follows:

- **Auxiliary Data Structures**:
  - `std::vector<std::vector<std::size_t>> dominated_by_this_solution`: This vector would store, for each solution `solutions[i]`, a list of indices `j` such that `solutions[i]` dominates `solutions[j]`.
  - `std::vector<std::size_t> domination_count`: This vector would store, for each solution `solutions[i]`, the count of how many other solutions dominate `solutions[i]`.

- **Comparison Loop**:
  - A nested loop would iterate through all unique pairs of solutions `(solutions[i], solutions[j])`.
  - For each pair, `manager->is_strictly_better_than` (or `is_probably_better_than` for heuristic search) would be used to determine the dominance relationship.
  - Based on the dominance, `domination_count[j]` would be incremented if `solutions[i]` dominates `solutions[j]`, and `solutions[j]`'s index would be added to `dominated_by_this_solution[i]`. Similarly for the reverse.

- **Filtering**:
  - After the comparison loop, iterate through `domination_count`. All solutions `solutions[i]` where `domination_count[i]` is 0 are the non-dominated solutions.
  - These non-dominated solutions would then be moved into a new `std::vector<flexman::core::Solution<State, Resources>>`, which would then `swap` with the original `solutions` vector.

### Benefits

- **Standard Algorithm**: Utilizes a well-established algorithm for non-dominated sorting.
- **Genericity**: Works with the existing `Manager` interface (`is_strictly_better_than`), without requiring assumptions about the internal structure or number of objectives within the `Resources` type.
- **Foundation**: Provides a solid foundation for future enhancements, such as integrating with multi-objective evolutionary algorithms or exploring more advanced dominance-checking data structures if the number of objectives becomes very high and specific objective access is provided.

While the worst-case complexity remains `O(N^2)` for the initial comparison phase, this structured approach is generally more efficient in practice and is the standard way to handle non-dominated sorting in multi-objective optimization contexts. Further improvements beyond `O(N^2)` would typically require specific knowledge of the objective space (e.g., 2D, 3D) or a mechanism to expose objective values for specialized data structures (like k-d trees) or algorithms (like sweep-line).
