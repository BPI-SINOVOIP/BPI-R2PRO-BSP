import { TestBed } from '@angular/core/testing';

import { ResponseErrorService } from './response-error.service';

describe('ResponseErrorService', () => {
  let service: ResponseErrorService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(ResponseErrorService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
