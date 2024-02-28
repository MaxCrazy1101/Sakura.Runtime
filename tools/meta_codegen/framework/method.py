'''
Method json structure
{
    "name": <name: str>,
    "isStatic": <is_static: bool>,
    "isConst": <is_const: bool>,
    "isNothrow": <is_nothrow: bool>,
    "attrs": <user attributes: Object>,
    "comment": <comment: str>,
    "parameters": {
        <name: str>: {
            "type": <type: str>,
            "arraySize": <array_size: int>,
            "rawType": <raw_type: str>,
            "attrs": <user attributes: Object>,
            "isFunctor": <is_functor: bool>,
            "isCallback": <is_callback: bool>,
            "isAnonymous": <is_anonymous: bool>,
            "comment": <comment: str>,
            "offset": <offset: int>,
            "line": <line: int>,
        },
        ...
    },
    "retType": <return_type: str>,
    "rawRetType": <raw_return_type: str>,
    "line": <line: int>,
}
'''


from typing import Dict
from parameter import Parameter


class Method:
    def __init__(self) -> None:
        self.name: str
        self.is_static: bool
        self.is_const: bool
        self.is_nothrow: bool
        self.comment: str
        self.parameters: Dict[str, Parameter]
        self.ret_type: str
        self.raw_ret_type: str
        self.line: int
        self.generator_data: Dict[str, object]
