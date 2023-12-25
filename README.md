## High performant UUID generator

TLDR(might be erronous in some places):

1. UUID (Universally Unique IDentifiers) are a way of generating IDentifiers which is unique
   across space and time. They are unique globally even if the entity they were generated with,
   cease to exist. UUIDs are 128 bits long and they donot require a central registration process
   to uphold their uniqueness globally. Because of this quality, they are widely used everywhere.

2. UUIDs are of 128 bits(16 octets). There is no padding between octets. To minimize confusion between
   octets, fields are often chosen to be multiples of 8. Two or more non-related fields can be multiplexed(combined) to
   form a new field if the newly formed field is a multiple of 8.

3. Following are fields in a UUID:

| Field                     | Data Type      | Octet | Note                                                                 |
| ------------------------- | -------------- | ----- | -------------------------------------------------------------------- |
| time_low                  | unsigned long  | 0-3   | The low field of the timestamp.                                      |
| time_mid                  | unsigned short | 4-5   | The middle field of the timestamp.                                   |
| time_hi_and_version       | unsigned short | 6-7   | The high field of the timestamp multiplexed with the version number. |
| clock_seq_hi_and_reserved | unsigned small | 8     | The high field of the clock sequence multiplexed with the variant.   |
| clock_seq_low             | unsigned small | 9     | The low field of the clock sequence.                                 |
| node                      | character      | 10-15 | The spatially unique node identifier.                                |

4. `timestamp` is a 60-bit value which is number of 100ns intervals passed since 00:00:00.00 15 October 1582(the date of Gregorian Reform to Christian)
   For this timestamp, we generally consider the UTC time (We are free to consider other timezones as well, as long as all the systems generating the UUIDs
   use that same timezone; but it is not recommended since getting the UTC from a local timezone is just offsetting it by some value.)

5. `timestamp` value is broken into 3 parts: `time_low`(32 bits), `time_mid`(16 bits) and `time_hi_and_version`(16 bits). `time_low` represents
   the lower field of the timestamp. `time_mid` represents the middle field of the timestamp and `time_hi_and_version` is a multiplexed field of the
   fields `time_hi`(12 bits of higher field of timestamp) and `version` (4 bits for UUID version). This is done so that `time_hi_and_version` field is
   a multiple of 8.

6. Read about the [variant field](https://datatracker.ietf.org/doc/html/rfc4122#section-4.1.1)
   and [version field](https://datatracker.ietf.org/doc/html/rfc4122#autoid-8) from the RFC as it's self explanatory. Note that the
   `variant` field is multiplexed in the 3 MSB bits `clock_seq_hi_and_reserved` field.

7. The `node` field depends on the version of UUID. For **UUIDv1**, `node` field is the `IEEE 802 MAC address` of the host machine. IEEE 802 MAC addresses
   are 48 bit Universally Unique addresses. This ensures that UUIDs generated on different machines(having different Network cards) always produce different
   UUIDs. For systems without a IEEE 802 MAC address, a random value can be used. The lowest addressed
   octet (octet number 10) contains the global/local bit and the unicast/multicast bit, and is the first octet of the address
   transmitted on an 802.3 LAN. The multicast bit must be set in such addresses, in order that they will never conflict with addresses obtained from network cards.

8. `clock_seq` is the field which is used to ensure that UUIDs (UUIDv1 specifically) generated on the same machine(having same 802 MAC) are always
   unique even if the monotonicity of the system clock is broken intentionally or unintentionally. Basically, consider a scenario when your system generated
   a timestamp of 12345. All the timestamps that you will generate later must be greater than this value. But, incase, the system clock is set backwards, then
   we will be able to generate the timestamp 12345 again which may lead to collision of UUIDs which was set before the system clock was set backwards and after
   the system clock was set backwards. This `clock_seq` value ensures it does not happen.

9. Suppose we generate a UUID. We save this UUID in a global store. When we create another UUID, we check if the timestamp of the UUID in the global store is
   greater than the current timestamp. If it is indeed greater, it means the system clock is set backwards. In this case, just increment the clock sequence
   of the previous UUID by 1(modulo 16,384) and set it as the clock sequence for the currently generated UUID. Save this UUID in the global store. The clock sequence MUST
   be originally set to a random value when initialized(i.e empty global store or if global store has been corrupted). Also, if a `node` value changes between
   generation(let's say the network card is replaced so the system now has a different MAC address), then set the clock sequence to a random value to minimize the
   probability of collision. **The initial value of clock sequence must not be co-related with the node identifier**.

10. The 100 nanosecond granularity of time should prove sufficient even for bursts of UUID creation in the next generation of high-performance multiprocessors.
    If a system overruns the clock adjustment by requesting too many UUIDs within a single system clock tick, the UUID service may raise an exception, handled in
    a system or process-dependent manner either by:

    - terminating the requester
    - reissuing the request until it succeeds
    - stalling the UUID generator until the system clock catches up

11. This is my attempt at making a high performant UUID generator referring [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122)

Read [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122) and [DCE/OSF Standard](https://pubs.opengroup.org/onlinepubs/9629399/apdxa.htm)
for more information.
