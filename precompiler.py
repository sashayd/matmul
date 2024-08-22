from io import StringIO
import re
import itertools


class VarsContainer:
    def __init__(self):
        return


class PrecompilerInstance:
    def __init__(self):
        self._re_replace = re.compile(r'@([a-z]+|\[([^\]])*\])')
        self._vars = VarsContainer()
        self._func_dict = {'product': itertools.product,
                           'read': (lambda x: self._read(x))}
        self._output_stream = None
        self._lines = None
        self._current_line = None

    def add_var(self, var_name, var_value):
        setattr(self._vars, var_name, var_value)

    def _read(self, file_name):
        with open(file_name, 'r') as f:
            return eval(f.read(), dict())

    def _error(self, message):
        line_index, line_contents = self._current_line
        raise RuntimeError(f'Error on line {line_index+1}\n'
                           f'{line_contents}\n'
                           f'{message}')

    def __call__(self, input_stream):
        self._output_stream = StringIO()
        self._lines = iter(enumerate(input_stream.readlines()))
        for indx, line in self._lines:
            self._current_line = indx, line
            command, content = self._preprocess_line(line)
            self._process_line(command, content)
        result = self._output_stream
        self._output_stream = None
        self._lines = None
        self._current_line = None
        return result

    def _eval(self, expr):
        result = eval(expr, self._vars.__dict__ | self._func_dict)
        return result

    def _preprocess_line(self, line):
        stripped = line.strip(' \t\n')
        if stripped and stripped[0] == '@':
            command = stripped.split(' ')[0][1:]
            rest = stripped[(len(command)+2):]
            if command in ['comment', 'assert', 'set', 'for', 'endfor']:
                return command, rest
            else:
                self._error(f'Unrecognized command {command}.')
        else:
            return 'noncommand', line

    def _process_line(self, command, content):
        result = getattr(self, f'_{command}')(content)
        if result is not None:
            self._output_stream.write(result)

    def _replace(self, content):
        expr = content[1:]
        if expr[0] == '[':
            evaluation = self._eval(expr[1:-1])
        else:
            evaluation = self._eval(expr)
        # insert exception for not being able to convert to str?
        result = str(evaluation)
        return result

    def _noncommand(self, content):
        return self._re_replace.sub(
            (lambda m: self._replace(m.group(0))), content
            )

    def _comment(self, content):
        return None

    def _assert(self, content):
        assertion = self._eval(content)
        if not assertion:
            self._error(f'Assertion ({content}) failed.')
        return None

    def _set(self, content):
        splitted = content.split('=')
        if not len(splitted) == 2:
            self._error('Illegal @set format.')
        left = splitted[0]
        right = splitted[1]
        left_splitted = left.split(',')
        right_object = self._eval(right)
        if len(left_splitted) > 1:
            if not len(left_splitted) == len(right_object):
                self._error('Illegal @set format.')
        else:
            right_object = [right_object]
        for i, le in enumerate(left_splitted):
            le = le.strip(' \t')
            if not le.isalpha():
                self._error('Variable names must consist of letters only.')
            val = right_object[i]
            setattr(self._vars, le, val)
        return None

    def _for(self, content):
        error_str = 'Illegal @for format.'
        splitted = content.split(' in ')
        assert len(splitted) == 2, error_str
        left = splitted[0]
        right = splitted[1]
        right_object = self._eval(right)
        left_splitted = left.split(',')
        left_vars = []
        for le in left_splitted:
            le = le.lstrip(' ').rstrip(' ')
            if not le.isalpha():
                self._error('Variable names must consist of letters only.')
            left_vars.append(le)
        if not left_vars:
            self._error('No variable to loop over in @for.')
        lines_buffer = []
        endfor_reached = False
        for indx, line in self._lines:
            self._current_line = indx, line
            command, content = self._preprocess_line(line)
            if command == 'endfor':
                endfor_reached = True
                break
            elif command == 'for':
                self._error('Nested @for are not supported currently.')
            else:
                lines_buffer.append((indx, line, command, content))
        if not endfor_reached:
            self._error('Encountered @for without matching @endfor.')
        for x in right_object:
            if len(left_vars) > 1:
                if len(x) != len(left_vars):
                    self._error('Numbers of variables do not match in @for.')
                for var, val in zip(left_vars, x):
                    setattr(self._vars, var, val)
            else:
                setattr(self._vars, left_vars[0], x)
            for indx, line, command, content in lines_buffer:
                self._current_line = indx, line
                self._process_line(command, content)
        return None
