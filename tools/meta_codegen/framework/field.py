'''
Field json structure
<field_name> : {
    "type": <type: str>,
    "rawType": <raw_type: str>,
    "arraySize": <array_size: int>,
    "attrs": <user attributes: Object>,
    "isFunctor": <is_functor: bool>,
    "isAnonymous": <is_anonymous: bool>,
    "comment": <comment: str>,
    "offset": <offset: int>,
    "line": <line: int>,
}
'''

from typing import Dict


class Field:
    def __init__(self) -> None:
        self.name: str
        self.type: str
        self.raw_type: str
        self.array_size: int
        self.is_functor: bool
        self.is_anonymous: bool
        self.comment: str
        self.offset: int
        self.line: int
        self.generator_data: Dict[str, object]
