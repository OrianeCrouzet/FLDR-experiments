# Released under Apache 2.0; refer to LICENSE.txt

from dataclasses import dataclass, field
from enum import IntEnum


class WeightedError(IntEnum):
    WEIGHTED_OK = 0
    WEIGHTED_NO_ITEM = 1
    WEIGHTED_TOO_MANY = 2
    WEIGHTED_INVALID_WEIGHT = 3
    WEIGHTED_ALL_ZERO = 4


@dataclass
class WeightedAlias:
    aliases: list = field(default_factory=list)
    prob: list = field(default_factory=list)
    small: list = field(default_factory=list)
    large: list = field(default_factory=list)
    n: int = 0
    weight_sum: float = 0


def pairwise_sum(weights, n=None):
    if n is None:
        n = len(weights)

    total = 0
    for weight in weights[:n]:
        total += weight
    return total


def weighted_alias_new(weights, n=None):
    if n is None:
        n = len(weights)

    if n == 0:
        return WeightedError.WEIGHTED_NO_ITEM, None

    dist = WeightedAlias(
        aliases=[0] * n,
        prob=[0] * n,
        small=[],
        large=[],
        n=n,
        weight_sum=0,
    )

    total = pairwise_sum(weights, n)
    if total == 0:
        return WeightedError.WEIGHTED_ALL_ZERO, None

    dist.weight_sum = total

    for i in range(n):
        p = weights[i] * n
        dist.prob[i] = p
        if p < total:
            dist.small.append(i)
        else:
            dist.large.append(i)

    while dist.small and dist.large:
        s = dist.small.pop()
        l = dist.large.pop()

        dist.aliases[s] = l
        dist.prob[l] = dist.prob[l] + dist.prob[s] - total

        if dist.prob[l] < total:
            dist.small.append(l)
        else:
            dist.large.append(l)

    while dist.large:
        l = dist.large.pop()
        dist.prob[l] = total

    while dist.small:
        s = dist.small.pop()
        dist.prob[s] = total

    return WeightedError.WEIGHTED_OK, dist


def alias_rust_preprocess(weights):
    err, dist = weighted_alias_new(weights)
    if err != WeightedError.WEIGHTED_OK:
        raise ValueError('weighted_alias_new failed with error %r' % (err,))
    return dist


def weighted_alias_sample(dist, rng_index, rng_weight):
    i = rng_index(dist.n)
    r = rng_weight(dist.weight_sum)
    if r < dist.prob[i]:
        return i
    return dist.aliases[i]


def weighted_alias_free(dist):
    dist.aliases = []
    dist.prob = []
    dist.small = []
    dist.large = []
    dist.n = 0
    dist.weight_sum = 0
